#pragma once

#include <cstddef>
#include <cstdint>
#include <type_traits>
enum class PipelineType{
    Base,
    Outline,
    Count
};

enum class RenderPassType{
    Base,
    Outline,
    ImGui,
    Count
};

enum class InstanceType : uint32_t{
    Polygon,
    Circle,
    Count
};

enum class DescriptorType{
    UniformBuffer,
    CombinedImageSampler,
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
    Count
};

template <typename T>
requires std::is_enum_v<T>
constexpr size_t toIndex(T type){
    return static_cast<size_t>(type);
}