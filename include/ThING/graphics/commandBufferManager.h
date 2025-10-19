#pragma once

#include <ThING/types/contexts.h>
#include <sys/types.h>
#include <vector>
#include <vulkan/vulkan_core.h>

class CommandBufferManager{
public:
    CommandBufferManager();
    void createCommandPool(VkPhysicalDevice& physicalDevice, VkDevice& device, VkSurfaceKHR& surface);
    void createCommandBuffers(VkDevice& device, VkSurfaceKHR& surface);
    void recordCommandBuffer(u_int32_t currentFrame, const PolygonContext& polygonContext, const CircleContext& circleContext, const FrameContext& frameContext);
    
    void cleanUpCommandBuffers(VkDevice& device);
    void cleanUpCommandPool(VkDevice& device);

    VkCommandPool& getCommandPool(){return commandPool;};
    VkCommandBuffer& getCommandBufferOnFrame(u_int32_t currentFrame){return commandBuffers[currentFrame];}; // Ocean Monument -1200 2830
private:
    void cmdSetBufferBeginInfo(VkCommandBuffer& commandBuffer);
    void cmdBeginRenderPass(VkCommandBuffer& commandBuffer, const FrameContext& frameContext);
    void cmdSetViewPort(VkCommandBuffer& commandBuffer, const FrameContext& frameContext);
    void cmdSetScissor(VkCommandBuffer& commandBuffer, const FrameContext& frameContext);
    void commandBindPipeline(VkCommandBuffer& commandBuffer, uint32_t currentFrame, const FrameContext& frameContext, PipelineType pipelineType);
    void cmdInitConfiguration(VkCommandBuffer& commandBuffer, const FrameContext& frameContext);
    void cmdEndConfiguration(VkCommandBuffer& commandBuffer);
    void recordPolygons(VkCommandBuffer& commandBuffer, const PolygonContext& polygonContext, uint32_t currentFrame, const FrameContext& frameContext);
    void recordCircles(VkCommandBuffer& commandBuffer, const CircleContext& circleContext, uint32_t currentFrame);

    std::vector<VkCommandBuffer> commandBuffers;
    VkCommandPool commandPool;
};