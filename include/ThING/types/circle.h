#pragma once

#include "glm/fwd.hpp"
#include <cstddef>
#include <cstdint>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <array>

struct Circle {
    glm::vec2 pos;
    float size;
    glm::vec3 color;
    float outlineSize = 0;
    glm::vec4 outlineColor = {0,0,0,1};
    uint32_t objectID = 0;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 1;
        bindingDescription.stride = sizeof(Circle);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

        return bindingDescription;
    }

    bool operator==(const Circle& a) const {
        return this->pos == a.pos && this->color == a.color && this->size == a.size;
    }

    static std::array<VkVertexInputAttributeDescription, 6> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 6> attributeDescriptions{};

        attributeDescriptions[0].binding = 1;
        attributeDescriptions[0].location = 2;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Circle, pos);
        
        attributeDescriptions[1].binding = 1;
        attributeDescriptions[1].location = 3;
        attributeDescriptions[1].format = VK_FORMAT_R32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Circle, size);

        attributeDescriptions[2].binding = 1;
        attributeDescriptions[2].location = 4;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Circle, color);

        attributeDescriptions[3].binding = 1;
        attributeDescriptions[3].location = 5;
        attributeDescriptions[3].format = VK_FORMAT_R32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Circle, outlineSize);

        attributeDescriptions[4].binding = 1;
        attributeDescriptions[4].location = 6;
        attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[4].offset = offsetof(Circle, outlineColor);

        attributeDescriptions[5].binding = 1;
        attributeDescriptions[5].location = 7;
        attributeDescriptions[5].format = VK_FORMAT_R32_UINT;
        attributeDescriptions[5].offset = offsetof(Circle, objectID);

        return attributeDescriptions;
    }
};