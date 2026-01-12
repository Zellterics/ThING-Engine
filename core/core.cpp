#include <ThING/core.h>
#include <ThING/extras/vulkanSupport.h>
#include <cstdint>

#include "ThING/graphics/bufferManager.h"
#include "ThING/graphics/pipelineManager.h"
#include "ThING/graphics/swapChainManager.h"
#include "ThING/types/contexts.h"
#include "ThING/types/enums.h"
#include "ThING/types/renderData.h"
#include "imgui.h"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "glm/fwd.hpp"

ProtoThiApp::ProtoThiApp() : windowManager(WIDTH, HEIGHT, "vulkan"){
    zoom = 1;
    offset = {0, 0};
    clearColor = {{{0.0f, 0.0f, 0.0f, 0.0f}}}; // STANDAR = BLACK
    currentFrame = 0;
    worldData.circleCount = 0;
    worldData.instances = {};
    worldData.meshes = {};
    worldData.polygonOffset = 0;
}

void ProtoThiApp::initVulkan() {
    createInstance();
    setupDebugMessenger();
    swapChainManager = SwapChainManager{instance, windowManager.getWindow()};
    pickPhysicalDevice();
    createLogicalDevice();
    commandBufferManager.createCommandPool(physicalDevice, device, swapChainManager.getSurface());
    swapChainManager.setDevice(device);
    swapChainManager.createSwapChain(physicalDevice, windowManager.getWindow());
    swapChainManager.createIdAttachments(physicalDevice);
    swapChainManager.createSeedAttachments(physicalDevice);
    pipelineManager.init(device, swapChainManager.viewImages()[0].format);
    swapChainManager.createFrameBuffers(pipelineManager.viewRenderPasses());
    swapChainManager.createJFAAttachments(physicalDevice);
    pipelineManager.createPipelines();
    bufferManager = BufferManager{device, physicalDevice, commandBufferManager.viewCommandPool(), graphicsQueue};
    bufferManager.createBuffers();
    pipelineManager.createDescriptors(bufferManager.viewBuffers(BufferType::Uniform), swapChainManager);
    commandBufferManager.createCommandBuffers(device, swapChainManager.getSurface());
    swapChainManager.createSyncObjects();
}

void ProtoThiApp::cleanup() {
    vkDeviceWaitIdle(device);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    swapChainManager.cleanUp();
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

    commandBufferManager.cleanUpCommandPool(device);

    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    
    vkDestroySurfaceKHR(instance, swapChainManager.getSurface(), nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwTerminate();
}

void ProtoThiApp::recordWorldData(std::span<InstanceData> circleInstances, std::span<InstanceData> polygonInstances, std::span<MeshData> meshes){
    worldData.instances.clear();
    
    worldData.circleCount = circleInstances.size();
    worldData.polygonOffset = worldData.circleCount;

    worldData.instances.reserve(circleInstances.size() + polygonInstances.size());

    worldData.instances.insert(worldData.instances.end(), circleInstances.begin(), circleInstances.end());
    worldData.instances.insert(worldData.instances.end(), polygonInstances.begin(), polygonInstances.end());

    worldData.meshes.assign(meshes.begin(), meshes.end());
    assert(worldData.meshes.size() == polygonInstances.size());
    for(int i = 0; i < polygonInstances.size(); i++){
        worldData.meshes[i].instanceIndex = worldData.polygonOffset + i;
    }
}

void ProtoThiApp::drawFrame() {
    vkWaitForFences(device, 1, &swapChainManager.getInFlightFences()[currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChainManager.getSwapChain(), UINT64_MAX, swapChainManager.getImageAvailableSemaphores()[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        swapChainManager.recreateSwapChain(physicalDevice, windowManager.getWindow(), pipelineManager.viewRenderPasses());
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(device, 1, &swapChainManager.getInFlightFences()[currentFrame]);

    vkResetCommandBuffer(commandBufferManager.viewCommandBufferOnFrame(currentFrame), 0);

    RenderContext renderContext = {currentFrame, worldData, bufferManager, indirectCommandCount};
    FrameContext frameContext{imageIndex, clearColor, pipelineManager, swapChainManager};
    pipelineManager.updateDescriptorSets(currentFrame, bufferManager.viewBuffer(BufferType::Uniform, currentFrame), swapChainManager, imageIndex);

    commandBufferManager.recordCommandBuffer(currentFrame, renderContext, frameContext);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {swapChainManager.getImageAvailableSemaphores()[currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBufferManager.viewCommandBufferOnFrame(currentFrame);

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
        swapChainManager.recreateSwapChain(physicalDevice, windowManager.getWindow(), pipelineManager.viewRenderPasses());
        pipelineManager.createDescriptors(bufferManager.viewBuffers(BufferType::Uniform), swapChainManager);
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}