#pragma once
#include "ThING/types/renderImage.h"
#include <span>
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
    void recreateSwapChain(VkPhysicalDevice& physicalDevice, GLFWwindow* window, std::span<const VkRenderPass> renderPasses);
    void createSurface(VkInstance& instance, GLFWwindow* window);
    
    void createDepthAttachments(VkPhysicalDevice physicalDevice);
    void createIdAttachments(VkPhysicalDevice physicalDevice);
    void createSeedAttachments(VkPhysicalDevice physicalDevice);
    void createJFAAttachments(VkPhysicalDevice physicalDevice);

    void cleanUp();
    void createFrameBuffers(std::span<const VkRenderPass> renderPass);
    void createSyncObjects();

    inline void setDevice(VkDevice device) {this->device = device;}

    inline VkSwapchainKHR getSwapChain() const {return swapChain;}
    inline const VkExtent2D& getExtent() const {return swapChainExtent;}
    inline VkSurfaceKHR& getSurface() {return surface;}
    inline std::vector<VkSemaphore>& getImageAvailableSemaphores() {return imageAvailableSemaphores;}
    inline std::vector<VkSemaphore>& getRenderFinishedSemaphores() {return renderFinishedSemaphores;}
    inline std::vector<VkFence>& getInFlightFences() {return inFlightFences;}

    inline std::span<const VkFramebuffer> viewBaseFrameBuffers() const {return baseFramebuffers;}
    inline std::span<const VkFramebuffer> viewImGuiFrameBuffers() const { return imGuiFramebuffers;}

    inline std::span<const RenderImage> viewImages() const {return images;}
    inline const RenderImage& viewIdImages() const {return idImages;}
    inline const RenderImage& viewSeedImages() const {return seedImages;}
    inline const RenderImage& viewJFAPingImages() const {return jfaPing;}
    inline const RenderImage& viewJFAPongImages() const {return jfaPong;}

    inline std::span<const VkFramebuffer> viewPostFrameBuffers() const {return postFramebuffers;}

private:
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

    uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void createBaseFramebuffers(const VkRenderPass& renderPass);
    void createPostFramebuffers(const VkRenderPass& renderPass);
    void createImGuiFramebuffers(const VkRenderPass& renderPass);

    void createImage(RenderImage& image, VkImageUsageFlags usage);
    void createImageMemory(RenderImage& image, VkPhysicalDevice physicalDevice);
    void createImageView(RenderImage& image);

    void createBaseImageViews();
    void createDepthImageView(RenderImage& image);

    VkDevice device;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    VkExtent2D swapChainExtent;

    std::vector<VkFramebuffer> baseFramebuffers;
    std::vector<VkFramebuffer> postFramebuffers;
    std::vector<VkFramebuffer> imGuiFramebuffers;

    std::vector<RenderImage> images;
    std::vector<RenderImage> depthImages;
    /**
     * @note idImages, seedImages, jfaPing, jfaPong are swapchain global by design,
     * as they are part of the per frame process of the Jump Flood Algorithm,
     * Duplicating these introduces per frame delay
     */
    RenderImage idImages;
    RenderImage seedImages;
    RenderImage jfaPing;
    RenderImage jfaPong;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
};