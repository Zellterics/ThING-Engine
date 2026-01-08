#include <ThING/extras/vulkanSupport.h>
#include <ThING/graphics/commandBufferManager.h>
#include <cstdint>
#include <stdexcept>
#include <vulkan/vulkan_core.h>
#include "ThING/graphics/bufferManager.h"
#include "ThING/graphics/pipelineManager.h"
#include "ThING/types/contexts.h"
#include "ThING/types/enums.h"
#include "ThING/types/renderData.h"
#include "backends/imgui_impl_vulkan.h"

CommandBufferManager::CommandBufferManager() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
}

void CommandBufferManager::createCommandPool(VkPhysicalDevice& physicalDevice, VkDevice& device, VkSurfaceKHR& surface) {
    QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics command pool!");
    }
}

void CommandBufferManager::createCommandBuffers(VkDevice& device, VkSurfaceKHR& surface) {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) commandBuffers.size();
    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void CommandBufferManager::cleanUpCommandBuffers(VkDevice& device){
    vkFreeCommandBuffers(device, commandPool,
        static_cast<uint32_t>(commandBuffers.size()),
        commandBuffers.data());
    commandBuffers.clear();
}

void CommandBufferManager::cleanUpCommandPool(VkDevice& device){
    vkDestroyCommandPool(device, commandPool, nullptr);
}

void CommandBufferManager::cmdSetBufferBeginInfo(VkCommandBuffer& commandBuffer){    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
}

void CommandBufferManager::cmdBeginBaseRenderPass(VkCommandBuffer& commandBuffer, const FrameContext& frameContext){
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = frameContext.pipelineManager.getRenderPasses()[toIndex(RenderPassType::Base)];
    renderPassInfo.framebuffer = frameContext.swapChainManager.getBaseFrameBuffers()[frameContext.imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = frameContext.swapChainManager.getExtent();

    renderPassInfo.clearValueCount = ATTACHMENT_COUNT;
    renderPassInfo.pClearValues = frameContext.clearColor;
    
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBufferManager::cmdBeginOutlineRenderPass(VkCommandBuffer& commandBuffer, const FrameContext& frameContext) {
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = frameContext.pipelineManager.getRenderPasses()[toIndex(RenderPassType::Outline)];
    renderPassInfo.framebuffer = frameContext.swapChainManager.getOutlineFrameBuffers()[frameContext.imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = frameContext.swapChainManager.getExtent();

    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = frameContext.clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBufferManager::cmdBeginImGuiRenderPass(VkCommandBuffer& commandBuffer, const FrameContext& frameContext) {
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = frameContext.pipelineManager.getRenderPasses()[toIndex(RenderPassType::ImGui)];
    renderPassInfo.framebuffer = frameContext.swapChainManager.getImGuiFrameBuffers()[frameContext.imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = frameContext.swapChainManager.getExtent();

    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = frameContext.clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBufferManager::cmdSetViewPort(VkCommandBuffer& commandBuffer, const FrameContext& frameContext){
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) frameContext.swapChainManager.getExtent().width;
    viewport.height = (float) frameContext.swapChainManager.getExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
}

void CommandBufferManager::cmdSetScissor(VkCommandBuffer& commandBuffer, const FrameContext& frameContext){
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = frameContext.swapChainManager.getExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void CommandBufferManager::commandBindPipeline(VkCommandBuffer& commandBuffer, uint32_t currentFrame, const FrameContext& frameContext, PipelineType type){
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, frameContext.pipelineManager.getGraphicsPipelines()[toIndex(type)]);

    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        frameContext.pipelineManager.getLayouts()[toIndex(type)],
        0, 1, &frameContext.pipelineManager.getDescriptorSets()[toIndex(type)][currentFrame],
        0, nullptr
    );
    
}

void CommandBufferManager::cmdInitRenderPass(VkCommandBuffer& commandBuffer, const FrameContext& frameContext, RenderPassType type){
    if(type == RenderPassType::Base){
        cmdBeginBaseRenderPass(commandBuffer, frameContext);
    }
    if(type == RenderPassType::Outline){
        cmdBeginOutlineRenderPass(commandBuffer, frameContext);
    }
    if(type == RenderPassType::ImGui){
        cmdBeginImGuiRenderPass(commandBuffer, frameContext);
    }
        cmdSetViewPort(commandBuffer, frameContext);
        cmdSetScissor(commandBuffer, frameContext);
}

void CommandBufferManager::cmdEndConfiguration(VkCommandBuffer& commandBuffer){
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CommandBufferManager::recordInstanceDraw(VkCommandBuffer& commandBuffer, const RenderContext& renderContext, const DrawBatch& drawBatch){
    VkBuffer vb[] = {
        renderContext.bufferManager.viewBuffer(drawBatch.vertexBuffer, 0).buffer, 
        renderContext.bufferManager.viewBuffer(BufferType::Instance, renderContext.currentFrame).buffer
    };
    VkDeviceSize offsets[] = {0,0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 2, vb, offsets);
    
    VkBuffer ib = renderContext.bufferManager.viewBuffer(drawBatch.indexBuffer, 0).buffer;
    vkCmdBindIndexBuffer(commandBuffer, ib, 0, VK_INDEX_TYPE_UINT16);

    vkCmdDrawIndexed(commandBuffer, drawBatch.indexCount, drawBatch.instanceCount, drawBatch.indexOffset, 0, drawBatch.instanceOffset);
}

void CommandBufferManager::recordIndirectDraw(VkCommandBuffer& commandBuffer, const RenderContext& renderContext, uint32_t commandCount){
    VkBuffer vb[] = {
        renderContext.bufferManager.viewBuffer(BufferType::Vertex, 0).buffer,
        renderContext.bufferManager.viewBuffer(BufferType::Instance, renderContext.currentFrame).buffer
    };

    VkDeviceSize offsets[] = {0, 0};

    vkCmdBindVertexBuffers(commandBuffer, 0, 2, vb, offsets);

    VkBuffer ib = renderContext.bufferManager.viewBuffer(BufferType::Index, 0).buffer;

    vkCmdBindIndexBuffer(commandBuffer, ib, 0, VK_INDEX_TYPE_UINT16);

    VkBuffer indirectBuffer = renderContext.bufferManager.viewBuffer(BufferType::Indirect, renderContext.currentFrame).buffer;

    vkCmdDrawIndexedIndirect(commandBuffer, indirectBuffer, 0, commandCount, sizeof(VkDrawIndexedIndirectCommand));
}


void CommandBufferManager::recordCommandBuffer(uint32_t currentFrame, const RenderContext& renderContext, const FrameContext& frameContext) {
    DrawBatch quadBatch = {
        .vertexBuffer = BufferType::QuadVertex,
        .indexBuffer = BufferType::QuadIndex,
        .indexCount = QUAD_INDICES.size(),
        .indexOffset = 0,
        .instanceCount = static_cast<uint32_t>(renderContext.worldData.circleCount),
        .instanceOffset = 0,
        
    };
    cmdSetBufferBeginInfo(commandBuffers[currentFrame]);
    cmdInitRenderPass(commandBuffers[currentFrame], frameContext, RenderPassType::Base);
        
        commandBindPipeline(commandBuffers[currentFrame], currentFrame, frameContext, PipelineType::Base);
        recordInstanceDraw(commandBuffers[currentFrame], renderContext, quadBatch);
        recordIndirectDraw(commandBuffers[currentFrame], renderContext, renderContext.indirectCmdCount);

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    cmdInitRenderPass(commandBuffers[currentFrame], frameContext, RenderPassType::Outline);

        commandBindPipeline(commandBuffers[currentFrame], currentFrame, frameContext, PipelineType::Outline);
        vkCmdDraw(commandBuffers[currentFrame], 3, 1, 0, 0);
    
    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    cmdInitRenderPass(commandBuffers[currentFrame], frameContext, RenderPassType::ImGui);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffers[currentFrame]);

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    cmdEndConfiguration(commandBuffers[currentFrame]);
}

