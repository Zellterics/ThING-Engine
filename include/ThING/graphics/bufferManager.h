#pragma once
#include "ThING/types/buffer.h"
#include "ThING/types/enums.h"
#include "ThING/types/renderData.h"
#include <array>
#include <cstddef>
#include <span>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <vector>
#include <ThING/consts.h>
#include <ThING/types/dynamicBuffer.h>
#include <ThING/types/vertex.h>
#include <ThING/types/uniformBufferObject.h>

class BufferManager{
public:
    BufferManager() = default;
    BufferManager(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
    void createBuffers();

    void updateCustomBuffers(std::span<Vertex> vertices, std::span<uint16_t> indices, WorldData& worldData, std::span<VkFence> inFlightFences, uint32_t frameIndex);
    void updateIndirectBuffers(std::span<const VkDrawIndexedIndirectCommand> commands, std::vector<VkFence>& inFlightFences, uint32_t frameIndex);
    void updateUniformBuffers(const VkExtent2D& swapChainExtent, float zoom, glm::vec2 offset, uint32_t frameIndex);
    void cleanUp();
    const Buffer& viewBuffer(BufferType type, size_t index) const;
    std::span<const Buffer, MAX_FRAMES_IN_FLIGHT> viewBuffers(BufferType type) const;
private:

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    void createCustomBuffers();
    void createIndirectBuffers();
    void createUniformBuffers();

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void uploadBuffer(VkDeviceSize bufferSize, VkBuffer *buffer, void* bufferData);
    void updateBuffer(VkFence& inFlightFences, const void* data, VkDeviceSize newBufferSize, 
        uint32_t frameIndex, VkBufferUsageFlags usage, BufferType type);

    Buffer& getBuffer(BufferType type, size_t index);
    std::array<Buffer, MAX_FRAMES_IN_FLIGHT>& getBuffers(BufferType type);

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


    std::vector<Buffer> buffers;
    VkDevice device;
    VkCommandPool commandPool;
    VkQueue graphicsQueue;
    VkPhysicalDevice physicalDevice;

    std::vector<DynamicBuffer<MAX_FRAMES_IN_FLIGHT>> stagingBuffers;
    UniformBufferObject ubo;
    Buffer ssbo;
    void* ssboMapped;
    std::array<void*, MAX_FRAMES_IN_FLIGHT> instancedMapped;
    
    std::array<Buffer, MAX_FRAMES_IN_FLIGHT> vertexBuffers;
    std::array<Buffer, MAX_FRAMES_IN_FLIGHT> indexBuffers;
    std::array<Buffer, MAX_FRAMES_IN_FLIGHT> uniformBuffers;
    Buffer quadVertexBuffer;
    Buffer quadIndexBuffer;
    std::array<Buffer, MAX_FRAMES_IN_FLIGHT> instanceBuffers;
    std::array<Buffer, MAX_FRAMES_IN_FLIGHT> indirectBuffers;
};