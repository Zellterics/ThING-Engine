#pragma once

#include <utility>
#include <vulkan/vulkan.h>

struct Buffer {
    VkBuffer buffer{VK_NULL_HANDLE};
    VkDeviceMemory memory{VK_NULL_HANDLE};
    VkDevice device{VK_NULL_HANDLE};

    Buffer() = default;
    Buffer(VkDevice d, VkBuffer b, VkDeviceMemory m) : buffer(b), memory(m), device(d) {}

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    Buffer(Buffer&& o) noexcept { *this = std::move(o); }
    Buffer& operator=(Buffer&& o) noexcept {
        if (this != &o) {
            destroy();
            device = o.device; buffer = o.buffer; memory = o.memory;
            o.device = VK_NULL_HANDLE; o.buffer = VK_NULL_HANDLE; o.memory = VK_NULL_HANDLE;
        }
        return *this;
    }

    ~Buffer() { destroy(); }

    void destroy() {
        if (device && buffer) vkDestroyBuffer(device, buffer, nullptr);
        if (device && memory) vkFreeMemory(device, memory, nullptr);
        buffer = VK_NULL_HANDLE; memory = VK_NULL_HANDLE; device = VK_NULL_HANDLE;
    }
};