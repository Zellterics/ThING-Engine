#pragma once
#include "ThING/types/quad.h"
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <vector>
#include <ThING/consts.h>
#include <ThING/types/dynamicBuffer.h>
#include <ThING/types/vertex.h>
#include <ThING/types/uniformBufferObject.h>
#include <ThING/types/circle.h>

class BufferManager{
public:
    BufferManager() = default;
    BufferManager(VkDevice device, VkPhysicalDevice physicalDevice, VkCommandPool commandPool, VkQueue graphicsQueue);
    void uploadBuffer(VkDeviceSize bufferSize, VkBuffer *buffer, void* bufferData);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void udpateBuffer(Buffer& passedBuffer, 
        VkFence& inFlightFences, 
        const void* data, 
        VkDeviceSize newBufferSize, 
        uint32_t frameIndex,
        VkBufferUsageFlags usage,
        size_t id);
    void createCustomBuffers(std::vector<Vertex>& vertices, 
        std::vector<uint16_t>& indices, 
        std::vector<Quad>& quadVertices,
        std::vector<uint16_t>& quadIndices,
        std::vector<Circle>& circleCenters);
    void updateCustomBuffers(std::vector<Vertex>& vertices, 
        std::vector<uint16_t>& indices, 
        std::vector<Circle>& circleCenters, 
        std::vector<VkFence>& inFlightFences,
        uint32_t frameIndex);
    void createUniformBuffers();
    void updateUniformBuffers(VkExtent2D& swapChainExtent, float zoom, glm::vec2 offset, uint32_t frameIndex);
    void cleanUp();

    Buffer* getVertexBuffers() {return vertexBuffers;};
    Buffer* getIndexBuffers() {return indexBuffers;};
    Buffer& getQuadBuffer() {return quadBuffer;};
    Buffer& getQuadIndexBuffer() {return quadIndexBuffer;};
    Buffer* getCircleBuffers() {return circleBuffers;};
    Buffer* getUniformBuffers() {return uniformBuffers;};
private:
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    std::vector<Buffer> buffers;
    VkDevice device;
    VkCommandPool commandPool;
    VkQueue graphicsQueue;
    VkPhysicalDevice physicalDevice;

    std::vector<DynamicBuffer<MAX_FRAMES_IN_FLIGHT>> stagingBuffers;
    UniformBufferObject ubo;
    Buffer vertexBuffers[MAX_FRAMES_IN_FLIGHT];
    Buffer indexBuffers[MAX_FRAMES_IN_FLIGHT];
    Buffer uniformBuffers[MAX_FRAMES_IN_FLIGHT];
    Buffer quadBuffer;
    Buffer circleBuffers[MAX_FRAMES_IN_FLIGHT];
    Buffer quadIndexBuffer;
};