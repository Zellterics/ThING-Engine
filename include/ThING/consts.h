#pragma once
#include "ThING/types/renderData.h"
#include "ThING/types/vertex.h"
#include <cstddef>
#include <cstdint>
#include <limits>

inline constexpr uint32_t WIDTH = 1200;
inline constexpr uint32_t HEIGHT = 800;
inline constexpr size_t MAX_FRAMES_IN_FLIGHT = 3;
inline constexpr size_t ATTACHMENT_COUNT = 3; //id, outline, base

inline constexpr int32_t MAX_INT_32 = std::numeric_limits<int32_t>::max();
inline constexpr float MAX_FLOAT = std::numeric_limits<float>::max();

//Polygon.h
inline constexpr const char* NULL_POLYGON_ID = "NULL_POLYGON_ID"; //REMOVE LATER IF NOT USED IN CORE
inline constexpr const char* NULL_CIRCLE_ID = "NULL_CIRCLE_ID"; //REMOVE LATER IF NOT USED IN CORE

//BufferManager.cpp

inline constexpr size_t BUFFER_PADDING = static_cast<size_t>(sizeof(Vertex)) * static_cast<size_t>(sizeof(InstanceData));
inline constexpr size_t MAX_INDIRECT_COMMANDS = 4096; // If you want more polygons just type more doesn't really matter