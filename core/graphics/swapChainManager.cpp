#include "ThING/extras/vulkanSupport.h"
#include "ThING/types/enums.h"
#include "ThING/types/renderImage.h"
#include <ThING/graphics/swapChainManager.h>
#include <algorithm>
#include <cstddef>
#include <span>
#include <stdexcept>
#include <vector>
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

    std::vector<VkImage> swapchainImages;
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapchainImages.data());
    
    for(size_t i = 0; i < imageCount; i++){
        images[i].image = swapchainImages[i];
        images[i].format = surfaceFormat.format;
        images[i].extent = extent;
    }

    swapChainExtent = extent;
    createBaseImageViews();
}

void SwapChainManager::recreateSwapChain(VkPhysicalDevice& physicalDevice, GLFWwindow* window, std::span<const VkRenderPass> renderPasses) {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);
    cleanUp();

    createSwapChain(physicalDevice, window);
    createIdAttachments(physicalDevice);
    createSeedAttachments(physicalDevice);
    createFrameBuffers(renderPasses);
    createJFAAttachments(physicalDevice);

    if (renderFinishedSemaphores.size() != images.size()) {
        for (auto s : renderFinishedSemaphores) vkDestroySemaphore(device, s, nullptr);
        renderFinishedSemaphores.clear();
        renderFinishedSemaphores.resize(images.size());
        VkSemaphoreCreateInfo si{}; si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        for (size_t i=0; i < renderFinishedSemaphores.size(); i++) {
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

void SwapChainManager::createImage(RenderImage& image, VkImageUsageFlags usage){
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width  = image.extent.width;
    imageInfo.extent.height = image.extent.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = image.format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.samples     = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &imageInfo, nullptr, &image.image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create ID image!");
    }
}

void SwapChainManager::createImageMemory(RenderImage& image, VkPhysicalDevice physicalDevice){
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image.image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice,memRequirements.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &image.Memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate ID image memory!");
    }

    vkBindImageMemory(device, image.image, image.Memory, 0);
}

void SwapChainManager::createImageView(RenderImage& image){
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = image.format;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &viewInfo, nullptr, &image.view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create ID image view!");
    }
}

void SwapChainManager::createJFAAttachments(VkPhysicalDevice physicalDevice){
    jfaPing.format = VK_FORMAT_R32G32_SFLOAT;
    jfaPing.extent = swapChainExtent;
    jfaPong.format = VK_FORMAT_R32G32_SFLOAT;
    jfaPong.extent = swapChainExtent;
    createImage(jfaPing, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    createImage(jfaPong, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT);
    createImageMemory(jfaPing, physicalDevice);
    createImageMemory(jfaPong, physicalDevice);
    createImageView(jfaPing);
    createImageView(jfaPong);
}

void SwapChainManager::createSeedAttachments(VkPhysicalDevice physicalDevice){
    seedImages.format = VK_FORMAT_R32G32_SFLOAT;
    seedImages.extent = swapChainExtent;
    createImage(seedImages, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    createImageMemory(seedImages, physicalDevice);
    createImageView(seedImages);
}

void SwapChainManager::createIdAttachments(VkPhysicalDevice physicalDevice) {
    idImages.format = VK_FORMAT_R32_UINT;
    idImages.extent = swapChainExtent;
    createImage(idImages, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT);
    createImageMemory(idImages, physicalDevice);
    createImageView(idImages);
}


void SwapChainManager::createBaseImageViews() {
    for (size_t i = 0; i < images.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = images[i].image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = images[i].format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &createInfo, nullptr, &images[i].view) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

void SwapChainManager::cleanUp(){
    for (auto framebuffer : baseFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    for (auto framebuffer : imGuiFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    for (auto framebuffer : postFramebuffers){
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    for (auto image : images) {
        vkDestroyImageView(device, image.view, nullptr);
    }
    vkDestroyImageView(device, idImages.view, nullptr);
    vkDestroyImage(device, idImages.image, nullptr);
    vkFreeMemory(device, idImages.Memory, nullptr);

    vkDestroyImageView(device, seedImages.view, nullptr);
    vkDestroyImage(device, seedImages.image, nullptr);
    vkFreeMemory(device, seedImages.Memory, nullptr);

    vkDestroyImageView(device, jfaPing.view, nullptr);
    vkDestroyImage(device, jfaPing.image, nullptr);
    vkFreeMemory(device, jfaPing.Memory, nullptr);

    vkDestroyImageView(device, jfaPong.view, nullptr);
    vkDestroyImage(device, jfaPong.image, nullptr);
    vkFreeMemory(device, jfaPong.Memory, nullptr);

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

void SwapChainManager::createFrameBuffers(std::span<const VkRenderPass> renderPasses){
    createBaseFramebuffers(renderPasses[toIndex(RenderPassType::Base)]);
    createImGuiFramebuffers(renderPasses[toIndex(RenderPassType::ImGui)]);
    createPostFramebuffers(renderPasses[toIndex(RenderPassType::Post)]);
}

void SwapChainManager::createBaseFramebuffers(const VkRenderPass& renderPass) {
    baseFramebuffers.resize(images.size());

    for (size_t i = 0; i < images.size(); i++) {
        std::array<VkImageView, 3> attachments = {
            images[i].view,
            idImages.view,
            seedImages.view
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = images[i].extent.width;
        framebufferInfo.height = images[i].extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &baseFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void SwapChainManager::createPostFramebuffers(const VkRenderPass& renderPass) {
    postFramebuffers.resize(images.size());

    for (size_t i = 0; i < images.size(); i++) {
        VkImageView attachments = {images[i].view};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &attachments;
        framebufferInfo.width = images[i].extent.width;
        framebufferInfo.height = images[i].extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &postFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void SwapChainManager::createImGuiFramebuffers(const VkRenderPass& renderPass) {
    imGuiFramebuffers.resize(images.size());
    for (size_t i = 0; i < images.size(); i++) {
        VkImageView attachments[] = { images[i].view };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = images[i].extent.width;
        framebufferInfo.height = images[i].extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &imGuiFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create imgui framebuffer!");
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