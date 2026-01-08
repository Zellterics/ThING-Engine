#pragma once
#include <cstddef>
#include <ThING/types/buffer.h>
#include <vulkan/vulkan_core.h>

template <std::size_t ARR_SIZE>
struct DynamicBuffer{
    VkDeviceSize bufferSizes[ARR_SIZE];
    Buffer stagingBuffer;
    bool isCreated = false;
    void* mappedData = nullptr;
    bool isMapped = false;
    DynamicBuffer(){
        for(VkDeviceSize& size : bufferSizes){
            size = 0;
        }
    }
};