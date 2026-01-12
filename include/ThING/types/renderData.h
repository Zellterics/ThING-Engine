#pragma once
#include "ThING/types/enums.h"
#include "ThING/types/vertex.h"
#include "glm/fwd.hpp"
#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#include <vector>

struct InstanceData {
    glm::vec2 position;
    glm::vec2 scale;

    float rotation = 0.0f;
    float outlineSize = 0.0f;
    uint32_t objectID = 0;
    InstanceType type;

    glm::vec4 color;
    glm::vec4 outlineColor = {0,0,0,0};

    uint32_t alive = 1;

    static std::array<VkVertexInputAttributeDescription, 9> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 9> attributes{};
        uint32_t loc = 2;
        uint32_t binding = 1;

        attributes[0] = { loc++, binding, VK_FORMAT_R32G32_SFLOAT, offsetof(InstanceData, position) };
        attributes[1] = { loc++, binding, VK_FORMAT_R32G32_SFLOAT, offsetof(InstanceData, scale) };

        attributes[2] = { loc++, binding, VK_FORMAT_R32_SFLOAT, offsetof(InstanceData, rotation) };
        attributes[3] = { loc++, binding, VK_FORMAT_R32_SFLOAT, offsetof(InstanceData, outlineSize) };
        attributes[4] = { loc++, binding, VK_FORMAT_R32_UINT, offsetof(InstanceData, objectID) };
        attributes[5] = { loc++, binding, VK_FORMAT_R32_UINT, offsetof(InstanceData, type) };

        attributes[6] = { loc++, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(InstanceData, color) };
        attributes[7] = { loc++, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(InstanceData, outlineColor) };

        attributes[8] = { loc++, binding, VK_FORMAT_R32_UINT, offsetof(InstanceData, alive) };

        return attributes;
    }

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription binding{};
        binding.binding = 1;
        binding.stride  = sizeof(InstanceData);
        binding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
        return binding;
    }
};


struct MeshData{
    uint32_t vertexOffset;
    uint32_t vertexCount;
    uint32_t indexOffset;
    uint32_t indexCount;
    uint32_t instanceIndex;

    constexpr MeshData() = delete; // NO NULL CONSTRUCTOR BECAUSE OF POSIBLE INDEX INCONSISTENCY
    MeshData(const uint32_t vertexOffset, const uint32_t vertexCount, const uint32_t indexOffset, const uint32_t indexCount, const uint32_t instanceIndex){
        this->vertexOffset = vertexOffset;
        this->vertexCount = vertexCount;
        this->indexOffset = indexOffset;
        this->indexCount = indexCount;
        this->instanceIndex = instanceIndex;
    }
    MeshData(const MeshData& mesh){
        this->vertexOffset = mesh.vertexOffset;
        this->vertexCount = mesh.vertexCount;
        this->indexOffset = mesh.indexOffset;
        this->indexCount = mesh.indexCount;
        this->instanceIndex = mesh.instanceIndex;
    }
    static constexpr unsigned long InstanceSize(){
        return sizeof(InstanceData);
    }
};

inline std::array<Vertex, 4> QUAD_VERTICES = {{
    {{-1.f, -1.f}, {-1.0f, -1.0f}},
    {{1.f, -1.f}, {1.0f, -1.0f}},
    {{1.f, 1.f}, {1.0f, 1.0f}},
    {{-1.f, 1.f}, {-1.0f, 1.0f}}
}};

inline std::array<glm::uint16_t, 6> QUAD_INDICES = {0,1,2,2,3,0};

struct DrawBatch {
    BufferType vertexBuffer;
    BufferType indexBuffer;
    uint32_t indexCount;
    uint32_t indexOffset;
    uint32_t instanceCount;
    uint32_t instanceOffset;
};

struct WorldData{
    std::vector<InstanceData> instances;
    std::vector<MeshData> meshes;

    uint32_t circleCount;
    uint32_t polygonOffset;
};