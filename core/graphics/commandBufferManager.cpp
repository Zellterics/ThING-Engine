#include <ThING/extras/vulkanSupport.h>
#include <ThING/graphics/commandBufferManager.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <vulkan/vulkan_core.h>
#include "ThING/consts.h"
#include "ThING/graphics/bufferManager.h"
#include "ThING/graphics/pipelineManager.h"
#include "ThING/types/contexts.h"
#include "ThING/types/enums.h"
#include "ThING/types/renderData.h"
#include "ThING/types/renderImage.h"
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
    renderPassInfo.renderPass = frameContext.pipelineManager.viewRenderPasses()[toIndex(RenderPassType::Base)];
    renderPassInfo.framebuffer = frameContext.swapChainManager.viewBaseFrameBuffers()[frameContext.imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = frameContext.swapChainManager.getExtent();

    renderPassInfo.clearValueCount = frameContext.clearColor.size();
    renderPassInfo.pClearValues = frameContext.clearColor.data();
    
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBufferManager::cmdBeginImGuiRenderPass(VkCommandBuffer& commandBuffer, const FrameContext& frameContext) {
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = frameContext.pipelineManager.viewRenderPasses()[toIndex(RenderPassType::ImGui)];
    renderPassInfo.framebuffer = frameContext.swapChainManager.viewImGuiFrameBuffers()[frameContext.imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = frameContext.swapChainManager.getExtent();

    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &frameContext.clearColor[0];

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
    VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkCmdBindPipeline(commandBuffer, bindPoint, frameContext.pipelineManager.viewPipelines()[toIndex(type)]);

    vkCmdBindDescriptorSets(
        commandBuffer,
        bindPoint,
        frameContext.pipelineManager.viewLayouts()[toIndex(type)],
        0, 1, &frameContext.pipelineManager.viewDescriptorSets()[toIndex(type)][currentFrame],
        0, nullptr
    );
    
}

void CommandBufferManager::cmdBeginPostRenderPass(VkCommandBuffer& commandBuffer, const FrameContext& frameContext){
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = frameContext.pipelineManager.viewRenderPasses()[toIndex(RenderPassType::Post)];
    renderPassInfo.framebuffer = frameContext.swapChainManager.viewPostFrameBuffers()[frameContext.imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = frameContext.swapChainManager.getExtent();

    renderPassInfo.clearValueCount = 0;
    renderPassInfo.pClearValues = nullptr;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}


void CommandBufferManager::cmdInitRenderPass(VkCommandBuffer& commandBuffer, const FrameContext& frameContext, RenderPassType type){
    switch (type) {
        case RenderPassType::Base:
            cmdBeginBaseRenderPass(commandBuffer, frameContext);
            break;
        case RenderPassType::ImGui:
            cmdBeginImGuiRenderPass(commandBuffer, frameContext);
            break;
        case RenderPassType::Post:
            cmdBeginPostRenderPass(commandBuffer, frameContext);
            break;
        case RenderPassType::Count: std::unreachable();
        default: std::unreachable();
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

    vkCmdDrawIndexed(commandBuffer, drawBatch.indexCount, 
        drawBatch.instanceCount, drawBatch.indexOffset, 0, drawBatch.instanceOffset);
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

void CommandBufferManager::cmdPipelineBarrier(VkCommandBuffer& commandBuffer, VkPipelineStageFlags srcStage, 
    VkPipelineStageFlags dstStage, VkImageMemoryBarrier& barrier){
    vkCmdPipelineBarrier(
        commandBuffer,
        srcStage,
        dstStage,
        0,
        0, 
        nullptr,
        0, 
        nullptr,
        1, 
        &barrier
    );
}

void CommandBufferManager::cmdBindComputePipeline(VkCommandBuffer& commandBuffer, const FrameContext& frameContext, uint32_t currentFrame) {
    vkCmdBindPipeline(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        frameContext.pipelineManager.viewPipelines()[toIndex(PipelineType::JFA)]
    );

    const VkDescriptorSet ds = frameContext.pipelineManager.viewJFADescriptorSets()[currentFrame];

    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_COMPUTE,
        frameContext.pipelineManager.viewLayouts()[toIndex(PipelineType::JFA)],
        0, 1, &ds,
        0, nullptr
    );
}

void CommandBufferManager::cmdDispatchJFA(VkCommandBuffer& commandBuffer, const FrameContext& frameContext) {
    constexpr uint32_t LOCAL = 16;

    const VkExtent2D extent = frameContext.swapChainManager.getExtent();
    const uint32_t gx = (extent.width  + LOCAL - 1) / LOCAL;
    const uint32_t gy = (extent.height + LOCAL - 1) / LOCAL;

    vkCmdDispatch(commandBuffer, gx, gy, 1);
}

void CommandBufferManager::recordJFAPass(VkCommandBuffer& commandBuffer, const FrameContext& frameContext, uint32_t currentFrame) {
    auto& ping = frameContext.swapChainManager.viewJFAPingImages();
    auto& pong = frameContext.swapChainManager.viewJFAPongImages();

    {
        VkImageMemoryBarrier barriers[2]{};

        for (size_t i = 0; i < 2; i++) {
            barriers[i].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barriers[i].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barriers[i].newLayout = VK_IMAGE_LAYOUT_GENERAL;
            barriers[i].srcAccessMask = 0;
            barriers[i].dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
            barriers[i].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barriers[i].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barriers[i].subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barriers[i].subresourceRange.baseMipLevel = 0;
            barriers[i].subresourceRange.levelCount = 1;
            barriers[i].subresourceRange.baseArrayLayer = 0;
            barriers[i].subresourceRange.layerCount = 1;
        }

        barriers[0].image = ping.image;
        barriers[1].image = pong.image;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0, 0, nullptr,
            0, nullptr,
            2, barriers
        );
    }

    const uint32_t w = frameContext.swapChainManager.getExtent().width;
    const uint32_t h = frameContext.swapChainManager.getExtent().height;
    const uint32_t maxDim = std::max(w, h);
    const uint32_t steps = (maxDim > 1) ? static_cast<uint32_t>(std::ceil(std::log2(double(maxDim)))) : 1;

    bool writeToPing = true;

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
frameContext.pipelineManager.viewPipelines()[toIndex(PipelineType::JFA)]);

    VkClearColorValue clear{};
    clear.float32[0] = 0.0f;
    clear.float32[1] = 0.0f;
    clear.float32[2] = 0.0f;
    clear.float32[3] = 0.0f;

    VkImageSubresourceRange range{};
    range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    range.baseMipLevel = 0;
    range.levelCount = 1;
    range.baseArrayLayer = 0;
    range.layerCount = 1;

    vkCmdClearColorImage(commandBuffer, ping.image, VK_IMAGE_LAYOUT_GENERAL, &clear, 1, &range);

    vkCmdClearColorImage(commandBuffer, pong.image, VK_IMAGE_LAYOUT_GENERAL, &clear, 1, &range);


    for (uint32_t s = 0; s < steps; s++) {
        int jump = 1 << (steps - 1 - s);

        int readFromPing;
        if (s == steps - 1) {
            readFromPing = 1;
        } else {
            readFromPing = writeToPing ? 1 : 0;
        }
            
        VkDescriptorSet ds = frameContext.pipelineManager.viewJFADescriptorSets()[currentFrame];

        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_COMPUTE,
            frameContext.pipelineManager.viewLayouts()[toIndex(PipelineType::JFA)],
            0,
            1,
            &ds,
            0,
            nullptr
        );

        std::array<int, 2> pcs = {jump, readFromPing};
        vkCmdPushConstants(
            commandBuffer,
            frameContext.pipelineManager.viewLayouts()[toIndex(PipelineType::JFA)],
            VK_SHADER_STAGE_COMPUTE_BIT,
            0,
            2 * sizeof(int),
            pcs.data()
        );

        constexpr uint32_t LOCAL = 16;
        vkCmdDispatch(
            commandBuffer,
            (w + LOCAL - 1) / LOCAL,
            (h + LOCAL - 1) / LOCAL,
            1
        );

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        barrier.image = (s == steps - 1) ? pong.image : (writeToPing ? ping.image : pong.image);

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
            0, 0, nullptr,
            0, nullptr,
            1, &barrier
        );

        if (s != steps - 1){
            writeToPing = !writeToPing;
        }
    }
    const VkImage resultImage = pong.image;
}


void CommandBufferManager::transitionImageToGeneral(VkCommandBuffer commandBuffer, const RenderImage& image, const FrameContext& frameContext) {
    VkImageMemoryBarrier b{};
    b.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    b.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    b.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    b.srcAccessMask = 0;
    b.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    b.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    b.image = image.image;
    b.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    b.subresourceRange.levelCount = 1;
    b.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &b
    );
}

void CommandBufferManager::recordCommandBuffer(uint32_t currentFrame, const RenderContext& renderContext, const FrameContext& frameContext) {
    DrawBatch quadBatch = {
        .vertexBuffer = BufferType::QuadVertex,
        .indexBuffer = BufferType::QuadIndex,
        .indexCount = QUAD_INDICES.size(),
        .indexOffset = 0,
        .instanceCount = static_cast<uint32_t>(renderContext.worldData.polygonOffset),
        .instanceOffset = 0,
    };
    cmdSetBufferBeginInfo(commandBuffers[currentFrame]);
    static int layoutsInitialized[MAX_FRAMES_IN_FLIGHT] = {};
    if (!layoutsInitialized[currentFrame]) {
        transitionImageToGeneral(commandBuffers[currentFrame], frameContext.swapChainManager.viewIdImages(), frameContext);
        transitionImageToGeneral(commandBuffers[currentFrame], frameContext.swapChainManager.viewSeedImages(), frameContext);
        transitionImageToGeneral(commandBuffers[currentFrame], frameContext.swapChainManager.viewJFAPingImages(), frameContext);
        transitionImageToGeneral(commandBuffers[currentFrame], frameContext.swapChainManager.viewJFAPongImages(), frameContext);
        layoutsInitialized[currentFrame] = true;
    }
    cmdInitRenderPass(commandBuffers[currentFrame], frameContext, RenderPassType::Base);
        
        commandBindPipeline(commandBuffers[currentFrame], currentFrame, frameContext, PipelineType::Base);
        recordInstanceDraw(commandBuffers[currentFrame], renderContext, quadBatch);
        recordIndirectDraw(commandBuffers[currentFrame], renderContext, renderContext.indirectCmdCount);

    vkCmdEndRenderPass(commandBuffers[currentFrame]);
    
    recordJFAPass(commandBuffers[currentFrame], frameContext, currentFrame);

    cmdInitRenderPass(commandBuffers[currentFrame], frameContext, RenderPassType::Post);
        
        commandBindPipeline(commandBuffers[currentFrame], currentFrame, frameContext, PipelineType::Post);
        vkCmdDraw(commandBuffers[currentFrame],3, 1, 0, 0);

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    cmdInitRenderPass(commandBuffers[currentFrame], frameContext, RenderPassType::ImGui);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffers[currentFrame]);

    vkCmdEndRenderPass(commandBuffers[currentFrame]);

    cmdEndConfiguration(commandBuffers[currentFrame]);
}

