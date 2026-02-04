#include "ThING/types/apiTypes.h"
#include "ThING/types/enums.h"
#include "ThING/types/renderData.h"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "miniaudio.h"
#include <ThING/types/vertex.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <span>
#include <sys/stat.h>
#include <utility>
#include <vector>
#define MINIAUDIO_IMPLEMENTATION
#include <ThING/api.h>
#include "imgui.h"

// Backends (GLFW + Vulkan)
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

//CONSTRUCTOR
ThING::API::API() : app(){
    if (ma_engine_init(nullptr, &audioEngine) != MA_SUCCESS){
        assert("ERROR: Initializing audio engine");
    }
    updateCallback = nullptr;
    uiCallback = nullptr;

    circleInstances = {};
    polygonInstances = {};
    polygonMeshes = {};

    circleFreeList = {};
    polygonFreeList = {};

    apiFlags = 0;

    volume = 255.f;
}

ThING::API::API(uint8_t flags) : API(){
    apiFlags = flags;
}

ThING::API::~API(){
    ma_engine_uninit(&audioEngine);
}

void ThING::API::run(){
    app.initVulkan();
    app.initImGui();
    mainLoop();
    app.cleanup();
}

bool ThING::API::setUpdateCallback(std::function<void(ThING::API&, FPSCounter&)> update){
    this->updateCallback = update;
    if(updateCallback == nullptr)
        return false;
    return true;
}

bool ThING::API::setUICallback(std::function<void(ThING::API&, FPSCounter&)> UI){
    this->uiCallback = UI;
    if(uiCallback == nullptr)
        return false;
    return true;
}

//PRIVATE
void ThING::API::mainLoop() {
    FPSCounter fps;
    while (!glfwWindowShouldClose(app.windowManager.getWindow())) {
        fps.beginFrame();

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        dirtyFlags.ssbo = false;
        dirtyFlags.meshes = false;

        // Callbacks
        if(apiFlags & ApiFlags_UpdateCallbackFirst){
            if(updateCallback) updateCallback(*this, fps);
            if(uiCallback) uiCallback(*this, fps);
        } else {
            if(uiCallback) uiCallback(*this, fps);
            if(updateCallback) updateCallback(*this, fps);
        }
        //RENDER
        ImGui::Render();
        app.recordWorldData(circleInstances, polygonInstances, std::span(reinterpret_cast<InstanceData*>(lineInstances.data()), 
            lineInstances.size()), polygonMeshes, dirtyFlags);
        app.renderFrame();
        
        fps.endFrame();
    }
    vkDeviceWaitIdle(app.device);
}

Entity ThING::API::addCircle(InstanceData&& instance){
    Entity e;
    if(circleFreeList.empty()){
        circleInstances.push_back(std::move(instance));
        e = {static_cast<uint32_t>(circleInstances.size() - 1), InstanceType::Circle};
        return e;
    } else {
        e = circleFreeList.back();
        circleFreeList.pop_back();
        circleInstances[e.index] = std::move(instance);
        return e;
    }
};

Entity ThING::API::addLine(LineData&& instance){
    Entity e;
    if(lineFreeList.empty()){
        lineInstances.push_back(std::move(instance));
        e = {static_cast<uint32_t>(lineInstances.size() - 1), InstanceType::Line};
        return e;
    } else {
        e = lineFreeList.back();
        lineFreeList.pop_back();
        lineInstances[e.index] = std::move(instance);
        return e;
    }
}

//PUBLIC

uint32_t ThING::API::getInstanceCount(InstanceType type){
    size_t count = 0;
    switch (type) {
        case InstanceType::Polygon:
            for(InstanceData instance : polygonInstances){
                if(instance.alive){
                    count++;
                }
            }
            return count;
        case InstanceType::Circle:
            for(InstanceData instance : circleInstances){
                if(instance.alive){
                    count++;
                }
            }
            return count;
        case InstanceType::Line:
            for(LineData instance : lineInstances){
                if(instance.alive){
                    count++;
                }
            }
            return count;
        case InstanceType::Count: std::unreachable();
        default: std::unreachable();
    }
}

void ThING::API::getWindowSize(int* x, int* y){
    glfwGetWindowSize(app.windowManager.getWindow(), x, y);
    return;
}

Entity ThING::API::addCircle(glm::vec2 pos, float size, glm::vec4 color){
    InstanceData tempInstance;
    tempInstance.position = pos;
    tempInstance.scale = {size, size};
    tempInstance.type = InstanceType::Circle;
    tempInstance.color = color;
    return addCircle(std::move(tempInstance));
}

std::span<InstanceData> ThING::API::getInstanceVector(InstanceType type){
    switch (type) {
        case InstanceType::Polygon: return polygonInstances;
        case InstanceType::Circle: return circleInstances;
        case InstanceType::Line: return {reinterpret_cast<InstanceData*>(lineInstances.data()), lineInstances.size()};
        case InstanceType::Count: std::unreachable();
        default: std::unreachable();
    }
}

std::span<LineData> ThING::API::getLineVector(){
    return lineInstances;
}

void ThING::API::setZoom(float zoom){
    app.zoom = zoom;
}

void ThING::API::setOffset(glm::vec2 offset){
    app.offset = offset;
}

void ThING::API::setBackgroundColor(glm::vec4 color){
    app.clearColor[0].color.float32[0] = color.x;
    app.clearColor[0].color.float32[1] = color.y;
    app.clearColor[0].color.float32[2] = color.z;
    app.clearColor[0].color.float32[3] = color.w;
}

Entity ThING::API::addPolygon(glm::vec2 pos, glm::vec4 color, glm::vec2 scale, std::span<Vertex> ver, std::span<uint16_t> ind){
    Entity e;

    InstanceData tempInstance;
    tempInstance.position = pos;
    tempInstance.scale = scale;
    tempInstance.type = InstanceType::Polygon;
    tempInstance.color = color;

    if(polygonFreeList.empty()){
        polygonInstances.push_back(std::move(tempInstance));
        e = {static_cast<uint32_t>(polygonInstances.size()) - 1, InstanceType::Polygon};
    } else {
        e = polygonFreeList.back();
        polygonFreeList.pop_back();
        polygonInstances[e.index] = std::move(tempInstance);
    }

    MeshData tempMesh = {
        to_u32(app.vertices.size()),
        to_u32(ver.size()),
        to_u32(app.indices.size()),
        to_u32(ind.size()),
        e.index
    };
    if(polygonMeshes.size() > e.index){
        polygonMeshes[e.index] = std::move(tempMesh);
    } else {
        polygonMeshes.push_back(std::move(tempMesh));
    }
    app.vertices.insert(app.vertices.end(), ver.begin(), ver.end());
    app.indices.insert(app.indices.end(), ind.begin(), ind.end());
    dirtyFlags.meshes = true;
    return e;
}

Entity ThING::API::addPolygon(glm::vec2 pos, glm::vec4 color, glm::vec2 scale, std::vector<Vertex>&& ver, std::vector<uint16_t>&& ind){
    Entity e;

    InstanceData tempInstance;
    tempInstance.position = pos;
    tempInstance.scale = scale;
    tempInstance.type = InstanceType::Polygon;
    tempInstance.color = color;
    tempInstance.drawIndex = 100;

    if(polygonFreeList.empty()){
        polygonInstances.push_back(std::move(tempInstance));
        e = {static_cast<uint32_t>(polygonInstances.size()) - 1, InstanceType::Polygon};
    } else {
        e = polygonFreeList.back();
        polygonFreeList.pop_back();
        polygonInstances[e.index] = std::move(tempInstance);
    }

    MeshData tempMesh = {
        to_u32(app.vertices.size()),
        to_u32(ver.size()),
        to_u32(app.indices.size()),
        to_u32(ind.size()),
        e.index
    };
    if(polygonMeshes.size() > e.index){
        polygonMeshes[e.index] = std::move(tempMesh);
    } else {
        polygonMeshes.push_back(std::move(tempMesh));
    }
    app.vertices.insert(app.vertices.end(), std::make_move_iterator(ver.begin()), std::make_move_iterator(ver.end()));
    app.indices.insert(app.indices.end(), std::make_move_iterator(ind.begin()), std::make_move_iterator(ind.end()));
    dirtyFlags.meshes = true;
    return e;
}

bool ThING::API::exists(const Entity e){
    if(e == INVALID_ENTITY){
        return false;
    }
    switch (e.type) {
        case InstanceType::Polygon:
            if(e.index >= polygonInstances.size()){
                return false;
            }
            if(!polygonInstances[e.index].alive){
                return false;
            }
            return true;
        case InstanceType::Circle:
            if(e.index >= circleInstances.size()){
                return false;
            }
            if(!circleInstances[e.index].alive){
                return false;
            }
            return true;
        case InstanceType::Line:
            if(e.index >= lineInstances.size()){
                return false;
            }
            if(!lineInstances[e.index].alive){
                return false;
            }
            return true;
        case InstanceType::Count: std::unreachable();
        default: std::unreachable();
    }
}

bool ThING::API::deleteInstance(const Entity e){
    if(!exists(e)){
        return false;
    }
    switch (e.type) {
        case InstanceType::Polygon:
            polygonInstances[e.index].alive = false;
            polygonInstances[e.index].objectID = 0;
            polygonFreeList.push_back(e);
            return true;
        case InstanceType::Circle:
            circleInstances[e.index].alive = false;
            circleInstances[e.index].objectID = 0;
            circleFreeList.push_back(e);
            return true;
        case InstanceType::Line:
            lineInstances[e.index].alive = false;
            lineInstances[e.index].objectID = 0;
            lineFreeList.push_back(e);
            return true;
        case InstanceType::Count: std::unreachable();
        default: std::unreachable();
    }
}

InstanceData& ThING::API::getInstance(const Entity e){
    switch (e.type) {
        case InstanceType::Polygon: 
            assert(e.index < polygonInstances.size() && "Invalid Entity passed to getInstance"); 
            return polygonInstances[e.index];
        case InstanceType::Circle: 
            assert(e.index < circleInstances.size() && "Invalid Entity passed to getInstance"); 
            return circleInstances[e.index];
        case InstanceType::Line:
            assert(e.index < lineInstances.size() && "Invalid Entity passed to getInstance");
            return *reinterpret_cast<InstanceData*>(&lineInstances[e.index]);
        case InstanceType::Count: std::unreachable();
        default: std::unreachable();
    }
}

LineData& ThING::API::getLine(const Entity e){
    if(e.type == InstanceType::Line){
        return lineInstances[e.index];
    }
    std::unreachable();
}

Entity ThING::API::addRegularPol(size_t sides, glm::vec2 pos, glm::vec2 scale, glm::vec4 color){
    if(sides < 3){
        return INVALID_ENTITY;
    }
    std::vector<Vertex> vertices;
    vertices.reserve(sides);
    for(int i = 0; i < sides; i++){
        glm::vec2 vtx = {sin((6.28 * i / (float)sides)), cos((6.28 * i / (float)sides))};
        vertices.push_back({vtx, {glm::normalize(vtx)}});
    }
    std::vector<uint16_t> indices;
    indices.reserve((sides - 2) * 3);
    for(int i = 0; i < sides - 2;){
        indices.push_back(0);
        indices.push_back(++i);
        indices.push_back(++i);
        i--;
    }
    return addPolygon(pos, color, scale, std::move(vertices), std::move(indices));
}

Entity ThING::API::addLine(glm::vec2 point1, glm::vec2 point2, float width){
    LineData tempInstance;
    tempInstance.point1 = point1;
    tempInstance.point2 = point2;
    tempInstance.thickness = width;
    tempInstance.type = InstanceType::Line;
    tempInstance.color = {1,1,0,1};
    return addLine(std::move(tempInstance));
}

bool ThING::API::playAudio(const std::string& soundFile){
    static ma_sound_group playVolumeGroup{};
    static uint8_t vol = 255.f;

    static bool first = true;
    if(first){
        ma_sound_group_init(&audioEngine, 0, nullptr, &playVolumeGroup);
        ma_sound_group_set_volume(&playVolumeGroup, glm::pow((float)volume / 255.f, 2));

        first = false;
    }
    if(vol != volume){
        vol = volume;
        ma_sound_group_set_volume(&playVolumeGroup, glm::pow((float)volume / 255.f, 2));
    }
    if(ma_engine_play_sound(&audioEngine, soundFile.c_str(), &playVolumeGroup) != MA_SUCCESS){
        return false;
    }
    return true;
}

bool ThING::API::playAudio(const std::string& soundFile, uint8_t volume){
    //Can be optimized by replaying instead of reloading, not high performance impact right now to bother
    //Do if you/I need to play a lot of sounds at high speeds
    static bool first = true;
    static ma_sound_group playVolumeGroup{};
    static uint8_t vol = volume;
    
    
    if(first){
        ma_sound_group_init(&audioEngine, 0, nullptr, &playVolumeGroup);
        ma_sound_group_set_volume(&playVolumeGroup, glm::pow((float)volume / 255.f, 2));

        first = false;
    }
    if(vol != volume){
        vol = volume;
        ma_sound_group_set_volume(&playVolumeGroup, glm::pow((float)volume / 255.f, 2));
    }
    if(ma_engine_play_sound(&audioEngine, soundFile.c_str(), &playVolumeGroup) != MA_SUCCESS){
        return false;
    }
    return true;
}

void ThING::API::clearInstanceVector(InstanceType type){
    switch (type) {
        case InstanceType::Circle:
            circleInstances.clear();
            circleFreeList.clear();
            break;
        case InstanceType::Line:
            lineInstances.clear();
            lineFreeList.clear();
            break;
        case InstanceType::Polygon:
            polygonInstances.clear();
            polygonMeshes.clear();
            polygonFreeList.clear();
            break;
        case InstanceType::Count:
            std::unreachable();
            break;
    }
}