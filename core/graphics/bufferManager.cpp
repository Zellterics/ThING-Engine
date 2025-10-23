#include "ThING/consts.h"
#include "ThING/types/vertex.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/fwd.hpp"
#include <ThING/graphics/bufferManager.h>
#include <cstddef>
#include <cstring>
#include <cstdint>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>

BufferManager::BufferManager(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue) 
: device(device), physicalDevice(physicalDevice), commandPool(commandPool), graphicsQueue(graphicsQueue) {
    ubo = {};
}

void BufferManager::uploadBuffer(VkDeviceSize bufferSize, VkBuffer *buffer, void* bufferData){
    if(bufferSize == 0){
        return;
    }
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, bufferData, (size_t) bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);
    copyBuffer(stagingBuffer, *buffer, bufferSize);
    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void BufferManager::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory){
    if(size == 0){
        size = 16;
    }
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(device, buffer, bufferMemory, 0);
}


void BufferManager::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size){
    if(size == 0){
        return;
    }
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

uint32_t BufferManager::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties){
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

void BufferManager::udpateBuffer(Buffer& passedBuffer, 
    VkFence& inFlightFences, 
    const void* data, 
    VkDeviceSize newBufferSize, 
    uint32_t frameIndex,
    VkBufferUsageFlags usage,
    size_t id){
    if(newBufferSize == 0){
        return;
    }
    if (stagingBuffers[id].bufferSizes[frameIndex] < newBufferSize){
        stagingBuffers[id].bufferSizes[frameIndex] = newBufferSize;
        if(passedBuffer.buffer){
            vkWaitForFences(device, 1, &inFlightFences, VK_TRUE, UINT64_MAX);
            passedBuffer.~Buffer();
            passedBuffer.device = device;
        }    
        createBuffer(stagingBuffers[id].bufferSizes[frameIndex], 
            usage, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
            passedBuffer.buffer, 
            passedBuffer.memory);
        if(stagingBuffers[id].stagingBuffer.buffer){
            vkUnmapMemory(device, stagingBuffers[id].stagingBuffer.memory);
            stagingBuffers[id].stagingBuffer.~Buffer();
            stagingBuffers[id].stagingBuffer.device = device;
            stagingBuffers[id].stagingBuffer = {};
            stagingBuffers[id].stagingBuffer.device = device;
            stagingBuffers[id].isMapped = false;
            stagingBuffers[id].isCreated = false;
            stagingBuffers[id].mappedData = nullptr;
        }
    }

    if (!stagingBuffers[id].isCreated){
        createBuffer(newBufferSize, 
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
            stagingBuffers[id].stagingBuffer.buffer, 
            stagingBuffers[id].stagingBuffer.memory);
        stagingBuffers[id].isCreated = true;
    }
    
    if(!stagingBuffers[id].isMapped){
        vkMapMemory(device, stagingBuffers[id].stagingBuffer.memory, 0, newBufferSize, 0, &stagingBuffers[id].mappedData);
        stagingBuffers[id].isMapped = true;
    }
    memcpy(stagingBuffers[id].mappedData, data, (size_t) newBufferSize);
    copyBuffer(stagingBuffers[id].stagingBuffer.buffer, passedBuffer.buffer, newBufferSize);
}

void BufferManager::updateCustomBuffers(std::vector<Vertex>& vertices, 
        std::vector<uint16_t>& indices, 
        std::vector<Circle>& circleCenters, 
        std::vector<VkFence>& inFlightFences,
        uint32_t frameIndex){
    if(stagingBuffers.size() < 3){
        stagingBuffers.emplace_back();
        stagingBuffers.emplace_back();
        stagingBuffers.emplace_back();
        stagingBuffers[0].stagingBuffer.device = device;
        stagingBuffers[1].stagingBuffer.device = device;
        stagingBuffers[2].stagingBuffer.device = device;
    }
    
    udpateBuffer(vertexBuffers[frameIndex], inFlightFences[frameIndex], vertices.data(), vertices.size() * sizeof(Vertex), frameIndex, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 0); //vertex
    udpateBuffer(indexBuffers[frameIndex], inFlightFences[frameIndex], indices.data(), indices.size() * sizeof(uint16_t), frameIndex, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 1);// index
    udpateBuffer(circleBuffers[frameIndex], inFlightFences[frameIndex], circleCenters.data(), circleCenters.size() * sizeof(Circle), frameIndex, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,2);// circle
}

void BufferManager::createUniformBuffers(){
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        void* data;
        uniformBuffers[i].device = device;
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i].buffer,uniformBuffers[i].memory);
    }
}

void BufferManager::updateUniformBuffers(VkExtent2D& swapChainExtent, float zoom, glm::vec2 offset, uint32_t frameIndex){
    const VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    static void* mappedData[MAX_FRAMES_IN_FLIGHT] = {nullptr};

    UniformBufferObject ubo{};
    static float width = 0;
    static float height = 0;

    width = (float) swapChainExtent.width;
    height = (float) swapChainExtent.height;
    if(zoom == 0){
        zoom = .001;
    }
    float halfWidth = (width / 2.0f) / zoom;
    float halfHeight = (height / 2.0f) / zoom;
    ubo.projection = glm::ortho(
        -halfWidth + offset.x, halfWidth + offset.x,
        -halfHeight + offset.y, halfHeight + offset.y,
        -1.0f, 1.0f
    );
    ubo.viewportSize = {swapChainExtent.width, swapChainExtent.height};
    if(!mappedData[frameIndex]){
        vkMapMemory(device, uniformBuffers[frameIndex].memory, 0, bufferSize, 0, &mappedData[frameIndex]);
    }
    
    memcpy(mappedData[frameIndex], &ubo, (size_t) bufferSize);
}

void BufferManager::createCustomBuffers(std::vector<Vertex>& vertices, 
        std::vector<uint16_t>& indices, 
        std::vector<Quad>& quadVertices,
        std::vector<uint16_t>& quadIndices,
        std::vector<Circle>& circleCenters){
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        vertexBuffers[i].device = device;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffers[i].buffer, vertexBuffers[i].memory);
        uploadBuffer(bufferSize, &vertexBuffers[i].buffer, vertices.data());
    }

    bufferSize = sizeof(indices[0]) * indices.size();
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        indexBuffers[i].device = device;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffers[i].buffer, indexBuffers[i].memory);
        uploadBuffer(bufferSize, &indexBuffers[i].buffer, indices.data());
    }

    bufferSize = sizeof(quadVertices[0]) * quadVertices.size();
    quadBuffer.device = device;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, quadBuffer.buffer, quadBuffer.memory);
    uploadBuffer(bufferSize, &quadBuffer.buffer, quadVertices.data());

    bufferSize = sizeof(quadIndices[0]) * quadIndices.size();
    quadIndexBuffer.device = device;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, quadIndexBuffer.buffer, quadIndexBuffer.memory);
    uploadBuffer(bufferSize, &quadIndexBuffer.buffer, quadIndices.data());

    bufferSize = sizeof(circleCenters[0]) * circleCenters.size();
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        circleBuffers[i].device = device;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, circleBuffers[i].buffer, circleBuffers[i].memory);
        uploadBuffer(bufferSize, &circleBuffers[i].buffer, circleCenters.data());
    }
}

void BufferManager::cleanUp(){
    quadBuffer.~Buffer();  
    quadIndexBuffer.~Buffer();
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vertexBuffers[i].~Buffer();
        indexBuffers[i].~Buffer();  
        uniformBuffers[i].~Buffer(); 
        circleBuffers[i].~Buffer();  
    }

    for (auto& dyn : stagingBuffers) {
        if (dyn.isMapped) {
            vkUnmapMemory(device, dyn.stagingBuffer.memory);
            dyn.isMapped = false;
            dyn.mappedData = nullptr;
        }
        if (dyn.stagingBuffer.buffer != VK_NULL_HANDLE)
            vkDestroyBuffer(device, dyn.stagingBuffer.buffer, nullptr);
        if (dyn.stagingBuffer.memory != VK_NULL_HANDLE)
            vkFreeMemory(device, dyn.stagingBuffer.memory, nullptr);
        dyn.stagingBuffer.buffer = VK_NULL_HANDLE;
        dyn.stagingBuffer.memory = VK_NULL_HANDLE;
    }
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (uniformBuffers[i].buffer != VK_NULL_HANDLE)
            vkDestroyBuffer(device, uniformBuffers[i].buffer, nullptr);
        if (uniformBuffers[i].memory != VK_NULL_HANDLE)
            vkFreeMemory(device, uniformBuffers[i].memory, nullptr);
    }
}