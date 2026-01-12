#pragma once
#include "ThING/types/renderData.h"
#include "ThING/types/vertex.h"
#include <cstddef>
#include <cstdint>

inline constexpr uint32_t WIDTH = 1200;
inline constexpr uint32_t HEIGHT = 800;
inline constexpr size_t MAX_FRAMES_IN_FLIGHT = 3;

//BufferManager.cpp
inline constexpr size_t BUFFER_PADDING = static_cast<size_t>(sizeof(Vertex)) * static_cast<size_t>(sizeof(InstanceData));
inline constexpr size_t MAX_INDIRECT_COMMANDS = 4096; // If you want more polygons just type more doesn't really matter