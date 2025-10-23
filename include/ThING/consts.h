#pragma once
#include "ThING/types/circle.h"
#include "ThING/types/polygon.h"
#include <cstddef>
#include <cstdint>

inline const uint32_t WIDTH = 1200;
inline const uint32_t HEIGHT = 800;
inline const size_t MAX_FRAMES_IN_FLIGHT = 3;
inline const size_t CURRENT_PIPELINES = 2;

//Polygon.h
inline const char* NULL_POLYGON_ID = "NULL_POLYGON_ID";
inline const Circle NULL_CIRCLE{{1000,1000}, 1000, {1000,1000,1000}};
inline const Polygon NULL_POLYGON{};