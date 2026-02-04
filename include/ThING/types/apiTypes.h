#pragma once

#include "ThING/types/enums.h"
#include <cassert>
#include <limits>
#include <functional>

struct Entity{
    uint32_t index;
    InstanceType type;
    bool operator ==(const Entity& other){
        if(this->index == other.index && this->type == other.type){
            return true;
        }
        return false;
    }
    bool operator ==(const Entity& other) const {
        if(this->index == other.index && this->type == other.type){
            return true;
        }
        return false;
    }
    bool operator !=(const Entity& other){
        if(this->index != other.index || this->type != other.type){
            return true;
        }
        return false;
    }
    bool operator !=(const Entity& other) const {
        if(this->index != other.index || this->type != other.type){
            return true;
        }
        return false;
    }
};

namespace std {
    template<>
    struct hash<Entity> {
        size_t operator()(const Entity& e) const noexcept {
            return (static_cast<size_t>(e.index) << 1) ^ static_cast<size_t>(e.type);
        }
    };
}

inline uint32_t to_u32(size_t v) {
    assert(v <= UINT32_MAX);
    return uint32_t(v);
}

constexpr Entity INVALID_ENTITY {
    std::numeric_limits<uint32_t>::max(),
    InstanceType::Count
};
