#pragma once

//it can happen
static_assert(sizeof(int) == 4, "Not Compatible with this compiler or configuration, int should be 4 bytes long");
static_assert(sizeof(float) == 4, "Not Compatible with this compiler or configuration, int should be 4 bytes long");

#include <ThING/types/vertex.h>
#include <ThING/types/uniformBufferObject.h>
#include <ThING/types/quad.h>
#include <ThING/types/circle.h>
#include <ThING/types/polygon.h>
#include <ThING/types/dynamicBuffer.h>
#include <ThING/types/buffer.h>
#include <ThING/types/contexts.h>
#include <ThING/types/enums.h>