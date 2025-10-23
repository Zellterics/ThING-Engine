#pragma once
#include <glm/glm.hpp>

namespace ThING{
    struct Collision{
        bool hit = 0;
        glm::vec2 normal = {0, 0};
        float depth = 0;
    };
}