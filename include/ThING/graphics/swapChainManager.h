#pragma once
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
    void recreateSwapChain(VkPhysicalDevice& physicalDevice, GLFWwindow* window, std::span<const VkRenderPass> renderPasses);//FIX ORDER OF FUNCTIONS
    void createImageViews();
    void createIdImageMemories(VkPhysicalDevice physicalDevice);
    void createOutlineDataImageMemories(VkPhysicalDevice physicalDevice);
    void createIdAttachments(VkPhysicalDevice physicalDevice);
    void createOutlineDataImageViews();
    void createOutlineDataImages();
    void createSurface(VkInstance& instance, GLFWwindow* window);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
    void cleanupSwapChain();
    void cleanUp();
    void createBaseFramebuffers(const VkRenderPass& renderPass);
    void createOutlineFramebuffers(const VkRenderPass& renderPass);
    void createImGuiFramebuffers(const VkRenderPass& renderPass);
    void createSyncObjects();

    inline void setDevice(VkDevice device) {this->device = device;};

    inline VkFormat& getImageFormat() {return imageFormat;};
    inline VkSwapchainKHR getSwapChain() const {return swapChain;}; // IDK if this thing is faster with & but I'll check that later
    inline const VkExtent2D& getExtent() const {return swapChainExtent;};
    inline std::vector<VkImage>& getImages() {return images;};
    inline VkSurfaceKHR& getSurface() {return surface;};
    inline std::vector<VkSemaphore>& getImageAvailableSemaphores() {return imageAvailableSemaphores;};
    inline std::vector<VkSemaphore>& getRenderFinishedSemaphores() {return renderFinishedSemaphores;};
    inline std::vector<VkFence>& getInFlightFences() {return inFlightFences;};
    inline std::span<const VkFramebuffer> getBaseFrameBuffers() const {return baseFramebuffers;};
    inline std::span<const VkFramebuffer> getOutlineFrameBuffers() const { return outlineFramebuffers; } // CHANGE TO VIEW NAME FOR CONSISNTENCY, Thanks.
    inline std::span<const VkFramebuffer> getImGuiFrameBuffers() const { return imGuiFramebuffers; }
    inline std::vector<VkImageView>& getIdImageViews() {return  idImageViews;};
    inline std::vector<VkImageView>& getOutlineDataImageViews() {return  outlineDataImageViews;};
    inline std::vector<VkImage>& getIdImages() {return  idImages;};

private:
    uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void createIdImages();
    void createIdImageViews();

    VkDevice device;
    // TOO MUCH IMAGES DEAR GOD FIND A WAY TO SIMPLIFY IT, A VECTOR OF VECTORS IS TOO UGLY, JUST FIND A WAY LATER, (ImageInfo Struct?)
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    VkFormat imageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkFramebuffer> baseFramebuffers;
    std::vector<VkFramebuffer> outlineFramebuffers;
    std::vector<VkFramebuffer> imGuiFramebuffers;

    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;

    std::vector<VkImage> idImages;
    std::vector<VkDeviceMemory> idImageMemories;
    std::vector<VkImageView> idImageViews;

    std::vector<VkImage> outlineDataImages;
    std::vector<VkDeviceMemory> outlineDataImageMemories;
    std::vector<VkImageView> outlineDataImageViews;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
};