#pragma once

#include "ThING/types/polygon.h"
#include <ThING/graphics/pipelineManager.h>
#include <ThING/graphics/swapChainManager.h>
#include <ThING/graphics/bufferManager.h>
#include <cstdint>
#include <vector>

struct FrameContext {
    uint32_t imageIndex;
    VkClearValue* clearColor;
    PipelineManager& pipelineManager;
    SwapChainManager& swapChainManager;
};

struct RenderContext {
    uint32_t currentFrame;
    BufferManager& bufferManager;
};

struct PolygonContext{
    std::vector<Polygon>& polygons;
    Buffer* vertexBuffers;
    Buffer* indexBuffers;
};

struct CircleContext{
    std::vector<Circle>& circleCenters;
    std::vector<uint16_t>& quadIndices;
    Buffer& quadBuffer;
    Buffer& quadIndexBuffer;
    Buffer* circleBuffers;
};