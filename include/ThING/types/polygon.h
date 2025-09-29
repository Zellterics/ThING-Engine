#include <cstdint>
#include <glm/glm.hpp>
#include <string>
#include <ThING/consts.h>

struct Transform {
    glm::vec2 position;
    float rotation;
    glm::u8vec4 outline; //IMPLEMENT LATER
    glm::vec2 scale;
};

struct Polygon{
    std::string id;
    uint32_t vertexOffset;
    uint32_t vertexCount;
    uint32_t indexOffset;
    uint32_t indexCount;
    bool alive;
    Transform transform;
    static constexpr unsigned long PushConstantSize(){
        return sizeof(transform);
    }
};

