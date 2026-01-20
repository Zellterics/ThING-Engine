#include "ThING/consts.h"
#include "ThING/types/enums.h"
#include "ThING/types/renderData.h"
#include "ThING/types/vertex.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/fwd.hpp"
#include <ThING/graphics/bufferManager.h>
#include <cstddef>
#include <cstring>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>

BufferManager::BufferManager(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue) 
: device(device), physicalDevice(physicalDevice), commandPool(commandPool), graphicsQueue(graphicsQueue) {
    ubo = {};
    for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        instancedMapped[i] = nullptr;
    }
    ssboMapped = nullptr;
}

void BufferManager::createBuffers(){
    createCustomBuffers();
    createIndirectBuffers();
    createUniformBuffers();
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
    VkFence fence;
    VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    vkCreateFence(device, &fenceInfo, nullptr, &fence);

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, fence);
    vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);

    vkDestroyFence(device, fence, nullptr);
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

const Buffer& BufferManager::viewBuffer(BufferType type, size_t index) const{
    switch (type) {
        case BufferType::Vertex:        return vertexBuffers[index];
        case BufferType::Index:         return indexBuffers[index];
        case BufferType::Instance:      return instanceBuffers[index];
        case BufferType::QuadVertex:    return quadVertexBuffer;
        case BufferType::QuadIndex:     return quadIndexBuffer;
        case BufferType::Uniform:       return uniformBuffers[index];
        case BufferType::Indirect:      return indirectBuffers[index];
        case BufferType::SSBO:          return ssbo;
        case BufferType::Count:         std::unreachable();
    }
    std::unreachable();
}

std::span<const Buffer, MAX_FRAMES_IN_FLIGHT> BufferManager::viewBuffers(BufferType type) const{
    switch (type) {
        case BufferType::Vertex:        return vertexBuffers;
        case BufferType::Index:         return indexBuffers;
        case BufferType::Instance:      return instanceBuffers;
        case BufferType::QuadVertex:    std::unreachable();
        case BufferType::QuadIndex:     std::unreachable();
        case BufferType::Uniform:       return uniformBuffers;
        case BufferType::Indirect:      return indirectBuffers;
        case BufferType::SSBO:          std::unreachable();
        case BufferType::Count:         std::unreachable();
    }
    std::unreachable();
}

Buffer& BufferManager::getBuffer(BufferType type, size_t index){
    switch (type) {
        case BufferType::Vertex:        return vertexBuffers[index];
        case BufferType::Index:         return indexBuffers[index];
        case BufferType::Instance:      return instanceBuffers[index];
        case BufferType::QuadVertex:    return quadVertexBuffer;
        case BufferType::QuadIndex:     return quadIndexBuffer;
        case BufferType::Uniform:       return uniformBuffers[index];
        case BufferType::Indirect:      return indirectBuffers[index];
        case BufferType::SSBO:          return ssbo;
        case BufferType::Count:         std::unreachable();
    }
    std::unreachable();
}

std::array<Buffer, MAX_FRAMES_IN_FLIGHT>& BufferManager::getBuffers(BufferType type){
    switch (type) {
        case BufferType::Vertex:        return vertexBuffers;
        case BufferType::Index:         return indexBuffers;
        case BufferType::Instance:      return instanceBuffers;
        case BufferType::QuadVertex:    std::unreachable();
        case BufferType::QuadIndex:     std::unreachable();
        case BufferType::Uniform:       return uniformBuffers;
        case BufferType::Indirect:      return indirectBuffers;
        case BufferType::SSBO:          std::unreachable();
        case BufferType::Count:         std::unreachable();
    }
    std::unreachable();
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

void BufferManager::updateBuffer(VkFence& inFlightFences, 
    const void* data, 
    VkDeviceSize newBufferSize, 
    uint32_t frameIndex,
    VkBufferUsageFlags usage,
    BufferType type){
    Buffer& passedBuffer = getBuffer(type, frameIndex);
    size_t id = toIndex(type);
    if(newBufferSize == 0){
        return;
    }
    if (stagingBuffers[id].bufferSizes[frameIndex] < newBufferSize){
        stagingBuffers[id].bufferSizes[frameIndex] = newBufferSize + BUFFER_PADDING;
        if(passedBuffer.buffer){
            vkWaitForFences(device, 1, &inFlightFences, VK_TRUE, UINT64_MAX);
            if (passedBuffer.buffer) {
                vkWaitForFences(device, 1, &inFlightFences, VK_TRUE, UINT64_MAX);
                passedBuffer.destroy();
            }
            passedBuffer.device = device;
        }    
        createBuffer(stagingBuffers[id].bufferSizes[frameIndex], 
            usage, 
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
            passedBuffer.buffer, 
            passedBuffer.memory);
        if (stagingBuffers[id].stagingBuffer.buffer){
            vkUnmapMemory(device, stagingBuffers[id].stagingBuffer.memory);
            stagingBuffers[id].stagingBuffer.destroy();
            stagingBuffers[id].stagingBuffer = {};
            stagingBuffers[id].stagingBuffer.device = device;
            stagingBuffers[id].isMapped = false;
            stagingBuffers[id].isCreated = false;
            stagingBuffers[id].mappedData = nullptr;
        }
    }

    if (!stagingBuffers[id].isCreated){
        createBuffer(newBufferSize + BUFFER_PADDING, 
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

void BufferManager::createUniformBuffers(){
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    for(int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        void* data;
        uniformBuffers[i].device = device;
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i].buffer,uniformBuffers[i].memory);
    }
}

void BufferManager::updateUniformBuffers(const VkExtent2D& swapChainExtent, float zoom, glm::vec2 offset, uint32_t frameIndex){
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

void BufferManager::createIndirectBuffers() {
    VkDeviceSize maxCommands = sizeof(VkDrawIndexedIndirectCommand) * MAX_INDIRECT_COMMANDS;

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        indirectBuffers[i].device = device;
        VkBufferUsageFlags flags = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        VkMemoryPropertyFlags propertys = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        createBuffer(maxCommands, flags, propertys, indirectBuffers[i].buffer, indirectBuffers[i].memory);
    }
}

void BufferManager::updateIndirectBuffers(std::span<const VkDrawIndexedIndirectCommand> commands, std::vector<VkFence>& inFlightFences, uint32_t frameIndex) {
    VkDeviceSize size = commands.size() * sizeof(VkDrawIndexedIndirectCommand);

    if (size == 0) return;
    VkBufferUsageFlags flags = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    updateBuffer(inFlightFences[frameIndex], commands.data(), size, frameIndex, flags, BufferType::Indirect);
}

void BufferManager::createCustomBuffers(){
    VkBufferUsageFlags vertexFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VkBufferUsageFlags indexFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VkBufferUsageFlags instanceFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    VkMemoryPropertyFlags memoryFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    VkBufferUsageFlags ssboFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

    VkMemoryPropertyFlags ssboMemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;


    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        vertexBuffers[i].device = device;
        createBuffer(BUFFER_PADDING, vertexFlags, memoryFlags, vertexBuffers[i].buffer, vertexBuffers[i].memory);
    }

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        indexBuffers[i].device = device;
        createBuffer(BUFFER_PADDING, indexFlags, memoryFlags, indexBuffers[i].buffer, indexBuffers[i].memory);
    }

    quadVertexBuffer.device = device;
    createBuffer(sizeof(Vertex) * QUAD_VERTICES.size(), vertexFlags, memoryFlags, quadVertexBuffer.buffer, quadVertexBuffer.memory);

    quadIndexBuffer.device = device;
    createBuffer(sizeof(uint16_t) * QUAD_INDICES.size(), indexFlags, memoryFlags, quadIndexBuffer.buffer, quadIndexBuffer.memory);
    uploadBuffer(sizeof(Vertex) * QUAD_VERTICES.size(), &quadVertexBuffer.buffer, (void*)QUAD_VERTICES.data());
    uploadBuffer(sizeof(uint16_t) * QUAD_INDICES.size(), &quadIndexBuffer.buffer, (void*)QUAD_INDICES.data());

    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++){
        instanceBuffers[i].device = device;
        createBuffer(MAX_INSTANCED_OBJECTS, instanceFlags, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, instanceBuffers[i].buffer, instanceBuffers[i].memory);
        vkMapMemory(device, instanceBuffers[i].memory, 0, VK_WHOLE_SIZE, 0, &instancedMapped[i]);
    }
        ssbo.device = device;
        createBuffer(MAX_SSBO_OBJECTS, ssboFlags, ssboMemoryFlags, ssbo.buffer, ssbo.memory);
        vkMapMemory(device, ssbo.memory, 0, VK_WHOLE_SIZE, 0, &ssboMapped);
}


void BufferManager::updateCustomBuffers(std::span<Vertex> vertices, std::span<uint16_t> indices, WorldData& worldData, std::span<VkFence> inFlightFences, uint32_t frameIndex){
    if (stagingBuffers.size() < toIndex(BufferType::Count)) {
        stagingBuffers.resize(toIndex(BufferType::Count)); // CHANGE IF I HAVE TIME, SOME BUFFERS DON'T NEED STAGING BUT IT WORKS (ubo, ssbo, etc)
        for (auto& sb : stagingBuffers) {
            sb.stagingBuffer.device = device;
        }
    }
    static std::array<bool, MAX_FRAMES_IN_FLIGHT> pendingMeshes = {};
    if (worldData.dirtyFlags.meshes) {
        for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) pendingMeshes[i] = true;
    }

    VkDeviceSize vertexSize = vertices.size() * sizeof(Vertex);
    VkDeviceSize indexSize = indices.size() * sizeof(uint16_t);
    VkDeviceSize instanceSize = (worldData.polygonOffset + worldData.polygonInstances.size()) * sizeof(InstanceData);
    VkDeviceSize ssboSize = worldData.ssboData.size() * sizeof(SSBO);

    VkBufferUsageFlags vertexFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    VkBufferUsageFlags indexFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    VkBufferUsageFlags instanceFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    if(pendingMeshes[frameIndex]){
        updateBuffer(inFlightFences[frameIndex], vertices.data(), vertexSize, frameIndex, vertexFlags, BufferType::Vertex);
        updateBuffer(inFlightFences[frameIndex], indices.data(), indexSize, frameIndex, indexFlags, BufferType::Index);
        pendingMeshes[frameIndex] = false;
    }//updateBuffer(inFlightFences[frameIndex], instanceData.data(), instanceSize, frameIndex, instanceFlags, BufferType::Instance);
    if(instanceSize > 0){
        InstanceData* dst = reinterpret_cast<InstanceData*>(instancedMapped[frameIndex]);
        memcpy(dst, worldData.circleInstances.data(), worldData.circleInstances.size() * sizeof(InstanceData));
        dst += worldData.circleInstances.size();
        memcpy(dst, worldData.lineInstances.data(), worldData.lineInstances.size() * sizeof(InstanceData));
        dst += worldData.lineInstances.size();
        memcpy(dst, worldData.polygonInstances.data(), worldData.polygonInstances.size() * sizeof(InstanceData));
    }
    if(ssboSize > 0 && worldData.dirtyFlags.ssbo){
        memcpy(ssboMapped, worldData.ssboData.data(), ssboSize);
    }
}

void BufferManager::cleanUp(){
    quadVertexBuffer.destroy();
    quadIndexBuffer.destroy();

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vertexBuffers[i].destroy();
        indexBuffers[i].destroy();
        uniformBuffers[i].destroy();
        instanceBuffers[i].destroy();
        indirectBuffers[i].destroy();
    }
    ssbo.destroy();

    for (auto& dyn : stagingBuffers) {
        if (dyn.isMapped) {
            vkUnmapMemory(device, dyn.stagingBuffer.memory);
            dyn.isMapped = false;
            dyn.mappedData = nullptr;
        }
        dyn.stagingBuffer.destroy();
    }
    stagingBuffers.clear();
}