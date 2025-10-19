#pragma once
#include "glm/fwd.hpp"
#include <cstdint>
#include <glm/glm.hpp>
#include <string>
#include <ThING/consts.h>

struct Transform {
    glm::vec2 position;
    float rotation;
    float padding;
    //float outlineSize;
    //glm::vec4 outlineColor; // I REALLY DONT WANT TO ADD NORMALS FOT THIS
    glm::vec2 scale;
    Transform() = default;
    Transform(const glm::vec2& position, const float& rotation, const glm::vec2& scale){
        this->position = position;
        this->rotation = rotation;
        this->scale = scale;
        this->padding = 0;
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
    Polygon(){
        id = NULL_POLYGON_ID;
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
        return sizeof(transform);
    }
};

