#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

class SwapChainManager{
public:
    SwapChainManager() = default;
    SwapChainManager(VkInstance& instance, GLFWwindow* window);
    void createSwapChain(VkPhysicalDevice& physicalDevice, GLFWwindow* window);
    void recreateSwapChain(VkPhysicalDevice& physicalDevice, GLFWwindow* window, VkRenderPass& renderPass);
    void createImageViews();
    void createSurface(VkInstance& instance, GLFWwindow* window);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
    void cleanupSwapChain();
    void createFramebuffers(VkRenderPass& renderPass);
    void createSyncObjects();

    inline void setDevice(VkDevice device) {this->device = device;};

    inline VkFormat& getImageFormat() {return imageFormat;};
    inline VkSwapchainKHR getSwapChain() const {return swapChain;}; // IDK if this thing is faster with & but I'll check that later
    inline VkExtent2D& getExtent() {return swapChainExtent;};
    inline std::vector<VkImage>& getImages() {return images;};
    inline VkSurfaceKHR& getSurface() {return surface;};
    inline std::vector<VkSemaphore>& getImageAvailableSemaphores() {return imageAvailableSemaphores;};
    inline std::vector<VkSemaphore>& getRenderFinishedSemaphores() {return renderFinishedSemaphores;};
    inline std::vector<VkFence>& getInFlightFences() {return inFlightFences;};
    inline std::vector<VkFramebuffer>& getFrameBuffers() {return framebuffers;};
private:
    VkDevice device;

    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> images;
    VkFormat imageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> imageViews;
    std::vector<VkFramebuffer> framebuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
};