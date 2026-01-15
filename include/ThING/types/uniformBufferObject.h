#pragma once
#include "glm/fwd.hpp"
#include <glm/glm.hpp>

struct UniformBufferObject {
    glm::mat4 projection;
    glm::vec2 viewportSize;
};
