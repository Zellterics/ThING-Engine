#pragma once

#include "ThING/types/enums.h"
#include "ThING/types/renderData.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

struct PhysicsObject{
    glm::vec2 currentPos;
    glm::vec2 oldPos;
    glm::vec2 acceleration;

    void updatePos(float dt){
        const glm::vec2 velocity = (currentPos - oldPos);
        oldPos = currentPos;
        const float RESISTANCE = .3f;
        currentPos = currentPos + velocity + (acceleration - velocity * RESISTANCE) * dt * dt;
        acceleration = {0.0f, 0.0f};
    }

    void accelerate(glm::vec2 acc){
        acceleration += acc;
    }

};

struct Collision{
    bool hit = 0;
    glm::vec2 normal = {0, 0};
    float depth = 0;
};

inline Collision getCircleCollision(const InstanceData& circle1, const InstanceData& circle2){
    Collision hit;
    if(circle1.type != InstanceType::Circle || circle2.type != InstanceType::Circle){
        return *(new Collision);
    }
    const glm::vec2 collision = circle2.position - circle1.position;
    const float summedSize = circle1.scale.x + circle2.scale.x;
    const float dist2 = glm::dot(collision, collision);
    if(dist2 >= summedSize * summedSize)
        return {};

    if(dist2 <= 1e-3f)
        return {};

    
    const float invDist = glm::inversesqrt(dist2);
    const float dist = dist2 * invDist;
    hit.hit = true;
    hit.normal = collision * invDist;
    hit.depth = summedSize - dist;
    return hit;
}