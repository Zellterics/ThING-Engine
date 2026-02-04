#pragma once

#include "glm/fwd.hpp"
#define GLFW_INCLUDE_VULKAN
#include <glm/glm.hpp>
#ifdef DEBUG
    #include <iostream>
#endif

#include <ThING/graphics/bufferManager.h>
#include <ThING/graphics/pipelineManager.h>
#include <ThING/window/windowManager.h>
#include <ThING/graphics/swapChainManager.h>
#include <ThING/graphics/commandBufferManager.h>

namespace ThING{
    class API;
}

class ProtoThiApp {
public:
    ProtoThiApp();
    void run();

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

    WorldData worldData;
    uint32_t indirectCommandCount;
    /* I really don't like this being here, it is calculated once per frame in mainLoop, but since I need this info I need
        a variable here... I think I really need that renderer class more every day, but I really want to get this donde right now
          so that's just life, move it later to a renderer class later maybe... */

    // Api Variables
    float zoom;
    glm::vec2 offset;
    std::vector<VkClearValue> clearColor;

    void initVulkan();
    void initImGui();
    void mainLoop();
    
    void renderFrame();
    void cleanup();

    void recordWorldData(std::span<InstanceData> circleInstances, std::span<InstanceData> polygonInstances, 
        std::span<InstanceData> lineInstances, std::span<MeshData> meshes, DirtyFlags dirtyFlags);
    
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
    

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
        VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        #ifdef DEBUG
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        #endif
        return VK_FALSE;
    }
};