#include <ThING/extras/vulkanSupport.h>
#include <ThING/graphics/commandBufferManager.h>
#include "ThING/types/polygon.h"
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

void CommandBufferManager::cmdBeginRenderPass(VkCommandBuffer& commandBuffer, const FrameContext& frameContext){
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = frameContext.pipelineManager.getrenderPass();
    renderPassInfo.framebuffer = frameContext.swapChainManager.getFrameBuffers()[frameContext.imageIndex];
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

void CommandBufferManager::commandBindPipeline(VkCommandBuffer& commandBuffer, uint32_t currentFrame, const FrameContext& frameContext, PipelineType pipelineType){
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, frameContext.pipelineManager.getGraphicsPipelines()[pipelineType]);
    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        frameContext.pipelineManager.getLayouts()[pipelineType],
        0, 1, &frameContext.pipelineManager.getDescriptorSets()[currentFrame],
        0, nullptr
    );
}

void CommandBufferManager::cmdInitConfiguration(VkCommandBuffer& commandBuffer, const FrameContext& frameContext){
    cmdSetBufferBeginInfo(commandBuffer);
    cmdBeginRenderPass(commandBuffer, frameContext);
        cmdSetViewPort(commandBuffer, frameContext);
        cmdSetScissor(commandBuffer, frameContext);
}

void CommandBufferManager::cmdEndConfiguration(VkCommandBuffer& commandBuffer){
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void CommandBufferManager::recordPolygons(VkCommandBuffer& commandBuffer, const PolygonContext& polygonContext, uint32_t currentFrame, const FrameContext& frameContext){
    VkBuffer vb = polygonContext.vertexBuffers[currentFrame].buffer;
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vb, offsets);
    
    VkBuffer ib = polygonContext.indexBuffers[currentFrame].buffer;
    vkCmdBindIndexBuffer(commandBuffer, ib, 0, VK_INDEX_TYPE_UINT16);
    for (const auto& poly : polygonContext.polygons) {
        if (!poly.alive) continue;
        Transform pc = poly.transform;
        pc.drawOutline = 1.f;
        vkCmdPushConstants(
            commandBuffer,
            frameContext.pipelineManager.getLayouts()[PIPELINE_TYPE_POLYGON],
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            Polygon::PushConstantSize(),
            &pc
        );

        vkCmdDrawIndexed(
            commandBuffer,
            poly.indexCount,
            1,
            poly.indexOffset,
            poly.vertexOffset,
            0
        );
        pc.drawOutline = 0.f;
        vkCmdPushConstants(
            commandBuffer,
            frameContext.pipelineManager.getLayouts()[PIPELINE_TYPE_POLYGON],
            VK_SHADER_STAGE_VERTEX_BIT,
            0,
            Polygon::PushConstantSize(),
            &pc
        );

        vkCmdDrawIndexed(
            commandBuffer,
            poly.indexCount,
            1,
            poly.indexOffset,
            poly.vertexOffset,
            0
        );
    }
}

void CommandBufferManager::recordCircles(VkCommandBuffer& commandBuffer, const CircleContext& circleContext, uint32_t currentFrame){
    VkBuffer vb[] = {circleContext.quadBuffer.buffer, circleContext.circleBuffers[currentFrame].buffer};
    VkDeviceSize offsets[] = {0,0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 2, vb, offsets);
    
    VkBuffer ib = circleContext.quadIndexBuffer.buffer;
    vkCmdBindIndexBuffer(commandBuffer, ib, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(circleContext.quadIndices.size()), circleContext.circleCenters.size(), 0, 0, 0);
}

void CommandBufferManager::recordCommandBuffer(uint32_t currentFrame, const PolygonContext& polygonContext, const CircleContext& circleContext, const FrameContext& frameContext) {
    cmdInitConfiguration(commandBuffers[currentFrame], frameContext);
        
        commandBindPipeline(commandBuffers[currentFrame], currentFrame, frameContext, PIPELINE_TYPE_POLYGON);
        recordPolygons(commandBuffers[currentFrame], polygonContext, currentFrame, frameContext);
        commandBindPipeline(commandBuffers[currentFrame], currentFrame, frameContext, PIPELINE_TYPE_CIRCLE);
        recordCircles(commandBuffers[currentFrame], circleContext, currentFrame);

        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffers[currentFrame]);

    cmdEndConfiguration(commandBuffers[currentFrame]);
}

