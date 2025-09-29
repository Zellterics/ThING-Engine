#include "ThING/consts.h"
#include "ThING/types/buffer.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"
#include <ThING/core.h>
#include <cstdint>
#include <vector>
void ProtoThiApp::createCommandBuffers() {
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

void ProtoThiApp::recordCommandBuffer(VkCommandBuffer commandBuffer, 
    uint32_t imageIndex,
    Buffer* vertexBuffers,
    Buffer* indexBuffers,
    Buffer& quadBuffer,
    Buffer& quadIndexBuffer,
    Buffer* circleBuffers) {
     VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = pipelineManager.getrenderPass();
    renderPassInfo.framebuffer = swapChainManager.getFrameBuffers()[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainManager.getExtent();

    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) swapChainManager.getExtent().width;
        viewport.height = (float) swapChainManager.getExtent().height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapChainManager.getExtent();
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager.getGraphicsPipelines()[0]);
            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineManager.getLayouts()[0],
                0, 1, &pipelineManager.getDescriptorSets()[currentFrame],
                0, nullptr
            );

            VkBuffer vb = vertexBuffers[currentFrame].buffer;
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vb, offsets);
            
            VkBuffer ib = indexBuffers[currentFrame].buffer;
            vkCmdBindIndexBuffer(commandBuffer, ib, 0, VK_INDEX_TYPE_UINT16);
            for (const auto& poly : polygons) {
                if (!poly.alive) continue;

                vkCmdPushConstants(
                    commandBuffer,
                    pipelineManager.getLayouts()[0],
                    VK_SHADER_STAGE_VERTEX_BIT,
                    0,
                    Polygon::PushConstantSize(),
                    &poly.transform
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
        {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineManager.getGraphicsPipelines()[1]);
            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineManager.getLayouts()[1],
                0, 1, &pipelineManager.getDescriptorSets()[currentFrame],
                0, nullptr
            );

            VkBuffer vb[] = {quadBuffer.buffer, circleBuffers[currentFrame].buffer};
            VkDeviceSize offsets[] = {0,0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 2, vb, offsets);
            
            VkBuffer ib = quadIndexBuffer.buffer;
            vkCmdBindIndexBuffer(commandBuffer, ib, 0, VK_INDEX_TYPE_UINT16);
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(quadIndices.size()), circleCenters.size(), 0, 0, 0);
        }
    
        // -------- integración de ImGui --------
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

        // -------- fin --------
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}