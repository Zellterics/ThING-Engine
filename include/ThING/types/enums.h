#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>

enum class PipelineType{
    Base,//graphics first
    Post,
    JFA,// compute last
    Count
};

const uint32_t GRAPHICS_PIPELINE_COUNT = 2; // Just count the above :p
const uint32_t COMPUTE_PIPELINE_COUNT = 1; // Same here

enum class RenderPassType{
    Base,
    Post,
    ImGui,
    Count
};

enum class InstanceType : uint32_t{
    Polygon,
    Circle,
    Line,
    Count
};

enum class DescriptorType{
    UniformBuffer,
    CombinedImageSampler,
    StorageImage,
    StorageBuffer,
    Count
};

enum class BufferType{
    Vertex,
    Index,
    Instance,
    QuadVertex,
    QuadIndex,
    Uniform,
    Indirect,
    SSBO,
    Count
};

template <typename T>
requires std::is_enum_v<T>
constexpr size_t toIndex(T type){
    return static_cast<size_t>(type);
}