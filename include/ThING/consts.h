#pragma once
#include "ThING/types/renderData.h"
#include "ThING/types/vertex.h"
#include <cstddef>
#include <cstdint>
#include <string>

inline constexpr uint32_t WIDTH = 1200;
inline constexpr uint32_t HEIGHT = 800;
inline constexpr std::string TITLE = "Vulkan";
inline constexpr size_t MAX_FRAMES_IN_FLIGHT = 3;

//BufferManager.cpp
inline constexpr size_t BUFFER_PADDING = static_cast<size_t>(sizeof(Vertex)) * static_cast<size_t>(sizeof(InstanceData));
inline constexpr size_t MAX_INDIRECT_COMMANDS = 0x100000; //around 65000 If you want more polygons just type more doesn't really matter 
inline constexpr uint32_t MAX_SSBO_OBJECTS = 0x1000000 * sizeof(SSBO); // around 1 Million If you want more (it literally can't take more), type more