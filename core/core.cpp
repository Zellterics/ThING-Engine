#include <ThING/core.h>
#include <ThING/extras/vulkanSupport.h>

#include "ThING/graphics/bufferManager.h"
#include "ThING/graphics/pipelineManager.h"
#include "ThING/types/contexts.h"
#include "imgui.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"

//ERASE THIS FUNCTION WITH THE COMMENT BELLOW
glm::mat4 build2DTransform(glm::vec2 pos, float rotation, glm::vec2 scale) {
    glm::mat4 t(1.0f);
    t = glm::translate(t, glm::vec3(pos, 0.0f));
    t = glm::rotate(t, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    t = glm::scale(t, glm::vec3(scale, 1.0f));
    return t;
}

ProtoThiApp::ProtoThiApp() : windowManager(WIDTH, HEIGHT, "vulkan"){
    zoom = 1;
    offset = {0, 0};
    clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}}; // STANDAR = BLACK
    currentFrame = 0;

    quadVertices = {
        {{-1.f, -1.f}, {-1.0f, -1.0f}},
        {{1.f, -1.f}, {1.0f, -1.0f}},
        {{1.f, 1.f}, {1.0f, 1.0f}},
        {{-1.f, 1.f}, {-1.0f, 1.0f}}
    };
    quadIndices = {0,1,2,2,3,0};
}



void ProtoThiApp::initVulkan() {
    createInstance();
    setupDebugMessenger();
    swapChainManager = SwapChainManager{instance, windowManager.getWindow()};
    pickPhysicalDevice();
    createLogicalDevice();
    swapChainManager.setDevice(device);
    swapChainManager.createSwapChain(physicalDevice, windowManager.getWindow());
    swapChainManager.createImageViews();
    pipelineManager.init(device, swapChainManager.getImageFormat());
    swapChainManager.createFramebuffers(pipelineManager.getrenderPass());
    pipelineManager.createPipelines();
    commandBufferManager.createCommandPool(physicalDevice, device, swapChainManager.getSurface());
    bufferManager = BufferManager{device, physicalDevice, commandBufferManager.getCommandPool(), graphicsQueue};
    bufferManager.createCustomBuffers(vertices, indices, quadVertices, quadIndices, circleCenters);
    bufferManager.createUniformBuffers();
    pipelineManager.createDescriptors(bufferManager.getUniformBuffers());
    commandBufferManager.createCommandBuffers(device, swapChainManager.getSurface());
    swapChainManager.createSyncObjects();
}

void ProtoThiApp::cleanup() {
    vkDeviceWaitIdle(device);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    swapChainManager.cleanupSwapChain();
    pipelineManager.cleanUp();
    vkDestroyDescriptorPool(device, imguiDescriptorPool, nullptr);

    commandBufferManager.cleanUpCommandBuffers(device);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        
        vkDestroySemaphore(device, swapChainManager.getImageAvailableSemaphores()[i], nullptr);
        vkDestroyFence(device, swapChainManager.getInFlightFences()[i], nullptr);
    }
    for(auto& semaphore : swapChainManager.getRenderFinishedSemaphores()){
        vkDestroySemaphore(device, semaphore, nullptr);
    }

    bufferManager.cleanUp();
    // while(!graphicsPipelines.empty()){
    //     vkDestroyPipeline(device, graphicsPipelines.back(), nullptr);
    //     graphicsPipelines.pop_back();
    // }

    // while(!pipelineLayouts.empty()){
    //     vkDestroyPipelineLayout(device, pipelineLayouts.back(), nullptr);
    //     pipelineLayouts.pop_back();
    // }

    commandBufferManager.cleanUpCommandPool(device);

    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    
    vkDestroySurfaceKHR(instance, swapChainManager.getSurface(), nullptr);
    vkDestroyInstance(instance, nullptr);

    

    glfwTerminate();
}

void ProtoThiApp::drawFrame() {
    vkWaitForFences(device, 1, &swapChainManager.getInFlightFences()[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChainManager.getSwapChain(), UINT64_MAX, swapChainManager.getImageAvailableSemaphores()[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        swapChainManager.recreateSwapChain(physicalDevice, windowManager.getWindow(), pipelineManager.getrenderPass());
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(device, 1, &swapChainManager.getInFlightFences()[currentFrame]);

    vkResetCommandBuffer(commandBufferManager.getCommandBufferOnFrame(currentFrame), /*VkCommandBufferResetFlagBits*/ 0);
    PolygonContext polygonContext = {polygons, bufferManager.getVertexBuffers(), bufferManager.getIndexBuffers()};
    CircleContext circleContext = {circleCenters, quadIndices, bufferManager.getQuadBuffer(), bufferManager.getQuadIndexBuffer(), bufferManager.getCircleBuffers()};
    FrameContext frameContext{imageIndex, &clearColor, pipelineManager, swapChainManager};
    commandBufferManager.recordCommandBuffer(currentFrame, polygonContext, circleContext, frameContext);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {swapChainManager.getImageAvailableSemaphores()[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBufferManager.getCommandBufferOnFrame(currentFrame);

    VkSemaphore signalSemaphores[] = {swapChainManager.getRenderFinishedSemaphores()[imageIndex]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, swapChainManager.getInFlightFences()[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChainManager.getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || windowManager.resizedFlag) {
        windowManager.resizedFlag = false;
        swapChainManager.recreateSwapChain(physicalDevice, windowManager.getWindow(), pipelineManager.getrenderPass());
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}