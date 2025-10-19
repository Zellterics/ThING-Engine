#pragma once

#include "glm/fwd.hpp"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>
#include <vector>
#include <cstdint>

#include <ThING/consts.h>
#include <ThING/core/detail.h>
#include <ThING/graphics/bufferManager.h>
#include <ThING/graphics/pipelineManager.h>
#include <ThING/window/windowManager.h>
#include <ThING/graphics/swapChainManager.h>
#include <ThING/graphics/commandBufferManager.h>
#include <ThING/extras/handMade.h>
#include <ThING/extras/fpsCounter.h>
#include <ThING/types.h>

namespace ThING{
    class API;
}

class ProtoThiApp {
public:
    ProtoThiApp();
    void run();

    //detail
    friend void ThING::detail::setResizedFlag(ProtoThiApp& app, bool flag);
    friend class ::ThING::API;
private:
    WindowManager windowManager;
    BufferManager bufferManager;
    PipelineManager pipelineManager;
    SwapChainManager swapChainManager;
    CommandBufferManager commandBufferManager;

    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;

    VkQueue graphicsQueue;
    VkQueue presentQueue;    

    uint32_t currentFrame;

    VkDescriptorPool imguiDescriptorPool;

    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    std::vector<Quad> quadVertices;
    std::vector<Circle> circleCenters;
    std::vector<uint16_t> quadIndices;

    // Api Variables
    float zoom;
    glm::vec2 offset;
    bool framebufferResized;
    VkClearValue clearColor;
    std::vector<Polygon> polygons;

    void initVulkan();
    void initImGui();
    void mainLoop();
    
    void renderFrame();
    void cleanup();
    
    void createInstance();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void setupDebugMessenger();
    void pickPhysicalDevice();
    void createLogicalDevice();

    void drawFrame();
    
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        #ifdef DEBUG
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        #endif
        return VK_FALSE;
    }
};