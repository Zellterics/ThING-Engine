#pragma once

#include "ThING/types/renderImage.h"
#include <ThING/types/contexts.h>
#include <sys/types.h>
#include <vulkan/vulkan_core.h>

class CommandBufferManager{
public:
    CommandBufferManager();
    void createCommandPool(VkPhysicalDevice& physicalDevice, VkDevice& device, VkSurfaceKHR& surface);
    void createCommandBuffers(VkDevice& device, VkSurfaceKHR& surface);
    void recordCommandBuffer(uint32_t currentFrame, const RenderContext& renderContext, const FrameContext& frameContext);
    
    void cleanUpCommandBuffers(VkDevice& device);
    void cleanUpCommandPool(VkDevice& device);

    const VkCommandPool& viewCommandPool(){return commandPool;};
    const VkCommandBuffer& viewCommandBufferOnFrame(uint32_t currentFrame){return commandBuffers[currentFrame];};
private:
    void cmdSetBufferBeginInfo(VkCommandBuffer& commandBuffer);
    void cmdInitRenderPass(VkCommandBuffer& commandBuffer, const FrameContext& frameContext, RenderPassType type);
    void cmdBeginBaseRenderPass(VkCommandBuffer& commandBuffer, const FrameContext& frameContext);
    void cmdBeginImGuiRenderPass(VkCommandBuffer& commandBuffer, const FrameContext& frameContext);
    void cmdBeginPostRenderPass(VkCommandBuffer& commandBuffer, const FrameContext& frameContext);
    void cmdEndConfiguration(VkCommandBuffer& commandBuffer);

    void transitionImageToGeneral(VkCommandBuffer commandBuffer, const RenderImage& image, const FrameContext& frameContext);

    void cmdSetViewPort(VkCommandBuffer& commandBuffer, const FrameContext& frameContext);
    void cmdSetScissor(VkCommandBuffer& commandBuffer, const FrameContext& frameContext);
    void commandBindPipeline(VkCommandBuffer& commandBuffer, uint32_t currentFrame, const FrameContext& frameContext, PipelineType type);

    void recordInstanceDraw(VkCommandBuffer& commandBuffer, const RenderContext& renderContext, const DrawBatch& drawBatch);
    void recordIndirectDraw(VkCommandBuffer& commandBuffer, const RenderContext& renderContext, uint32_t commandCount);

    void recordJFAPass(VkCommandBuffer& commandBuffer, const FrameContext& frameContext, uint32_t currentFrame);
    void cmdDispatchJFA(VkCommandBuffer& commandBuffer, const FrameContext& frameContext);
    void cmdBindComputePipeline(VkCommandBuffer& commandBuffer, const FrameContext& frameContext, uint32_t currentFrame);
    void cmdPipelineBarrier(VkCommandBuffer& commandBuffer, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage, VkImageMemoryBarrier& barrier);

    std::vector<VkCommandBuffer> commandBuffers;
    VkCommandPool commandPool;
};