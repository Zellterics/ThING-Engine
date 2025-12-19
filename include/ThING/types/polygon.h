#pragma once
#include "glm/fwd.hpp"
#include <cstdint>
#include <glm/glm.hpp>
#include <string>

struct Transform {
    glm::vec2 position;
    float rotation;
    float outlineSize;
    float drawOutline;
    float windingSign;
    glm::vec2 scale;
    glm::vec4 outlineColor;
    uint32_t objectID;
    Transform() = default;
    Transform(const glm::vec2& position, const float& rotation, const glm::vec2& scale){
        this->position = position;
        this->rotation = rotation;
        this->scale = scale;
        this->outlineSize = 0;
        this->outlineColor = {0,0,0,0};
        this->drawOutline = 0;
        this->windingSign = 0;
    }
    Transform(const glm::vec2& position, const float& rotation, const glm::vec2& scale, float outlineSize, glm::vec4 outlineColor, float windingSign){
        this->position = position;
        this->rotation = rotation;
        this->scale = scale;
        this->outlineSize = outlineSize;
        this->outlineColor = outlineColor;
        this->drawOutline = 1;
        this->windingSign = windingSign;
    }
    Transform(const glm::vec2& position, const float& rotation, const glm::vec2& scale, float outlineSize, glm::vec4 outlineColor, float windingSign, uint32_t objectID){
        this->position = position;
        this->rotation = rotation;
        this->scale = scale;
        this->outlineSize = outlineSize;
        this->outlineColor = outlineColor;
        this->drawOutline = 1;
        this->windingSign = windingSign;
        this->objectID = objectID;
    }
};

struct Polygon{
    std::string id;
    uint32_t vertexOffset;
    uint32_t vertexCount;
    uint32_t indexOffset;
    uint32_t indexCount;
    Transform transform;
    bool alive;
    constexpr Polygon(){
        id = "NULL_POLYGON_ID";
        vertexOffset = 0;
        vertexCount = 0;
        indexOffset = 0;
        indexCount = 0;
        transform = {{0,0}, 0, {0,0}};
        alive = false;
    }
    Polygon(
        const std::string& id, 
        const uint32_t& vertexOffset, 
        const uint32_t& vertexCount, 
        const uint32_t& indexOffset, 
        const uint32_t& indexCount, 
        const Transform& transform){
        this->id = id;
        this->vertexOffset = vertexOffset;
        this->vertexCount = vertexCount;
        this->indexOffset = indexOffset;
        this->indexCount = indexCount;
        this->transform = transform;
        this->alive = true;
    }
    static constexpr unsigned long PushConstantSize(){
        return sizeof(Transform);
    }
};

