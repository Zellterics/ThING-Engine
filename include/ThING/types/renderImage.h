#pragma once

#include <vulkan/vulkan_core.h>

struct RenderImage{
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory Memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkFormat format;
    VkExtent2D extent;
};