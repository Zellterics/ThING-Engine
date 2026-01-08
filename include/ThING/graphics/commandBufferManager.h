#pragma once

#include "ThING/types/renderData.h"
#include <ThING/types/contexts.h>
#include <sys/types.h>
#include <vector>
#include <vulkan/vulkan_core.h>

class CommandBufferManager{
public:
    CommandBufferManager();
    void createCommandPool(VkPhysicalDevice& physicalDevice, VkDevice& device, VkSurfaceKHR& surface);
    void createCommandBuffers(VkDevice& device, VkSurfaceKHR& surface);
    void recordCommandBuffer(u_int32_t currentFrame, const RenderContext& renderContext, const FrameContext& frameContext);
    
    void cleanUpCommandBuffers(VkDevice& device);
    void cleanUpCommandPool(VkDevice& device);

    VkCommandPool& getCommandPool(){return commandPool;};
    VkCommandBuffer& getCommandBufferOnFrame(u_int32_t currentFrame){return commandBuffers[currentFrame];};
private:
    void cmdSetBufferBeginInfo(VkCommandBuffer& commandBuffer);
    void cmdBeginBaseRenderPass(VkCommandBuffer& commandBuffer, const FrameContext& frameContext);
    void cmdBeginOutlineRenderPass(VkCommandBuffer& commandBuffer, const FrameContext& frameContext);
    void cmdBeginImGuiRenderPass(VkCommandBuffer& commandBuffer, const FrameContext& frameContext);
    void cmdSetViewPort(VkCommandBuffer& commandBuffer, const FrameContext& frameContext);
    void cmdSetScissor(VkCommandBuffer& commandBuffer, const FrameContext& frameContext);
    void commandBindPipeline(VkCommandBuffer& commandBuffer, uint32_t currentFrame, const FrameContext& frameContext, PipelineType type);
    void cmdInitRenderPass(VkCommandBuffer& commandBuffer, const FrameContext& frameContext, RenderPassType type);
    void cmdEndConfiguration(VkCommandBuffer& commandBuffer);
    void recordInstanceDraw(VkCommandBuffer& commandBuffer, const RenderContext& renderContext, const DrawBatch& drawBatch);
    void recordIndirectDraw(VkCommandBuffer& commandBuffer, const RenderContext& renderContext, uint32_t commandCount);
    std::vector<VkCommandBuffer> commandBuffers;
    VkCommandPool commandPool;
};