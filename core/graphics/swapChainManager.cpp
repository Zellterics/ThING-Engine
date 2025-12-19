#include "ThING/extras/vulkanSupport.h"
#include "ThING/types/enums.h"
#include <ThING/graphics/swapChainManager.h>
#include <algorithm>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include <limits>
#include <ThING/consts.h>
#include <cstdint>

SwapChainManager::SwapChainManager(VkInstance& instance, GLFWwindow* window){
    createSurface(instance, window);
}

void SwapChainManager::createSwapChain(VkPhysicalDevice& physicalDevice, GLFWwindow* window) {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;

    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    images.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, images.data());

    imageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void SwapChainManager::recreateSwapChain(VkPhysicalDevice& physicalDevice, GLFWwindow* window, std::span<VkRenderPass> renderPasses) {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);
    cleanUp();

    createSwapChain(physicalDevice, window);
    createImageViews();
    createIdAttachments(physicalDevice);
    createBaseFramebuffers(renderPasses[RENDER_PASS_TYPE_BASE]);
    createOutlineFramebuffers(renderPasses[RENDER_PASS_TYPE_OUTLINE]);
    createImGuiFramebuffers(renderPasses[RENDER_PASS_TYPE_IMGUI]);

    if (renderFinishedSemaphores.size() != images.size()) {
        for (auto s : renderFinishedSemaphores) vkDestroySemaphore(device, s, nullptr);
        renderFinishedSemaphores.clear();
        renderFinishedSemaphores.resize(images.size());
        VkSemaphoreCreateInfo si{}; si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        for (size_t i=0; i<renderFinishedSemaphores.size(); ++i) {
            if (vkCreateSemaphore(device, &si, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to recreate renderFinished semaphore!");
            }
        }
    }
}

uint32_t SwapChainManager::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void SwapChainManager::createIdImageViews(){
    VkFormat idFormat = VK_FORMAT_R32_UINT;
    idImageViews.resize(images.size());
    for(size_t i = 0; i < images.size(); i++){
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = idImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = idFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &viewInfo, nullptr, &idImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create ID image view!");
        }
    }
}

void SwapChainManager::createIdImages(){
    idImages.resize(images.size());
    VkFormat idFormat = VK_FORMAT_R32_UINT;
    for(size_t i = 0; i < images.size(); i++){
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width  = swapChainExtent.width;
        imageInfo.extent.height = swapChainExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = idFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage =
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples     = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(device, &imageInfo, nullptr, &idImages[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create ID image!");
        }
    }
}

void SwapChainManager::createIdImageMemories(VkPhysicalDevice physicalDevice){
    idImageMemories.resize(images.size());
    for(size_t i = 0; i < images.size(); i++){
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, idImages[i], &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex =
            findMemoryType(
                physicalDevice,
                memRequirements.memoryTypeBits,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );

        if (vkAllocateMemory(device, &allocInfo, nullptr, &idImageMemories[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate ID image memory!");
        }

        vkBindImageMemory(device, idImages[i], idImageMemories[i], 0);
    }
}

void SwapChainManager::createOutlineDataImageViews(){
    VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;
    outlineDataImageViews.resize(images.size());
    for(size_t i = 0; i < images.size(); i++){
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = outlineDataImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &viewInfo, nullptr, &outlineDataImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create ID image view!");
        }
    }
}

void SwapChainManager::createOutlineDataImages(){
    outlineDataImages.resize(images.size());
    VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT;
    for(size_t i = 0; i < images.size(); i++){
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width  = swapChainExtent.width;
        imageInfo.extent.height = swapChainExtent.height;
        imageInfo.extent.depth  = 1;
        imageInfo.mipLevels     = 1;
        imageInfo.arrayLayers   = 1;
        imageInfo.format        = format;
        imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage =
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples     = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(device, &imageInfo, nullptr, &outlineDataImages[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create ID image!");
        }
    }
}

void SwapChainManager::createOutlineDataImageMemories(VkPhysicalDevice physicalDevice){
    outlineDataImageMemories.resize(images.size());
    for(size_t i = 0; i < images.size(); i++){
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, outlineDataImages[i], &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex =
            findMemoryType(
                physicalDevice,
                memRequirements.memoryTypeBits,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            );

        if (vkAllocateMemory(device, &allocInfo, nullptr, &outlineDataImageMemories[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate ID image memory!");
        }

        vkBindImageMemory(device, outlineDataImages[i], outlineDataImageMemories[i], 0);
    }
}

void SwapChainManager::createIdAttachments(VkPhysicalDevice physicalDevice) {
    createIdImages();
    createIdImageMemories(physicalDevice);
    createIdImageViews();
    createOutlineDataImages();
    createOutlineDataImageMemories(physicalDevice);
    createOutlineDataImageViews();
}


void SwapChainManager::createImageViews() {
    imageViews.resize(images.size());

    for (size_t i = 0; i < images.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = images[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = imageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &createInfo, nullptr, &imageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }
}
// REFACTOR BOTH CLEANUPS LATER
void SwapChainManager::cleanupSwapChain() {
    for (auto framebuffer : baseFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    for (auto framebuffer : outlineFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    for (auto framebuffer : imGuiFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    for (auto imageView : imageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    for (auto imageView : idImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    for (auto imageView : outlineDataImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void SwapChainManager::cleanUp(){
    for (auto framebuffer : baseFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    for (auto framebuffer : outlineFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    for (auto framebuffer : imGuiFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    for (auto imageView : imageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    for (auto imageView : idImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    for (auto imageView : outlineDataImageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    for(auto image : idImages){
        vkDestroyImage(device, image, nullptr);
    }
    for(auto memory : idImageMemories){
        vkFreeMemory(device, memory, nullptr);
    }
    for(auto image : outlineDataImages){
        vkDestroyImage(device, image, nullptr);
    }
    for(auto memory : outlineDataImageMemories){
        vkFreeMemory(device, memory, nullptr);
    }
    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

VkSurfaceFormatKHR SwapChainManager::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR SwapChainManager::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}


void SwapChainManager::createSurface(VkInstance& instance, GLFWwindow* window) {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

VkExtent2D SwapChainManager::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void SwapChainManager::createBaseFramebuffers(VkRenderPass& renderPass) {
    baseFramebuffers.resize(imageViews.size());
    idImageViews.resize(imageViews.size()); //MAYBE ERASE

    for (size_t i = 0; i < imageViews.size(); i++) {
        std::array<VkImageView, 3> attachments = {
            imageViews[i],
            idImageViews[i],
            outlineDataImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &baseFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void SwapChainManager::createOutlineFramebuffers(VkRenderPass& renderPass) {
    outlineFramebuffers.resize(imageViews.size());
    outlineDataImageViews.resize(imageViews.size());

    for (size_t i = 0; i < imageViews.size(); i++) {
        VkImageView attachments[] = { imageViews[i] };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &outlineFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create outline framebuffer!");
        }
    }
}

void SwapChainManager::createImGuiFramebuffers(VkRenderPass& renderPass) {
    imGuiFramebuffers.resize(imageViews.size());
    for (size_t i = 0; i < imageViews.size(); i++) {
        VkImageView attachments[] = { imageViews[i] };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &imGuiFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create outline framebuffer!");
        }
    }
}

void SwapChainManager::createSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(images.size());

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }

    for (size_t i = 0; i < renderFinishedSemaphores.size(); i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create renderFinished semaphore for image!");
        }
    }
}