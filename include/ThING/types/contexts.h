#pragma once

#include "ThING/types/renderData.h"
#include <cstdint>
#include <span>
#include <vulkan/vulkan_core.h>

class PipelineManager;
class SwapChainManager;
class BufferManager;
struct InstanceData;
struct MeshData;

struct FrameContext {
    uint32_t imageIndex;
    std::span<VkClearValue> clearColor;
    const PipelineManager& pipelineManager;
    const SwapChainManager& swapChainManager;
};

struct RenderContext {
    uint32_t currentFrame;
    const WorldData worldData;
    const BufferManager& bufferManager;
    uint32_t indirectCmdCount;
};