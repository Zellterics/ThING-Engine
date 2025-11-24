#include "ThING/consts.h"
#include "ThING/types/circle.h"
#include "ThING/types/collission.h"
#include "ThING/types/polygon.h"
#include "glm/exponential.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "miniaudio.h"
#include <ThING/types/vertex.h>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <iostream>
#include <string>
#include <string_view>
#include <sys/stat.h>
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
}

ThING::API::~API(){

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
    this->uiCallback = UI;\
    if(uiCallback == nullptr)
        return false;
    return true;
}

//PRIVATE
void ThING::API::mainLoop() {
    FPSCounter fps;
    while (!glfwWindowShouldClose(app.windowManager.getWindow())) {
        
        //FRAME INIT
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Callbacks
        uiCallback(*this, fps);
        updateCallback(*this, fps);
        
        //RENDER
        ImGui::Render();
        app.renderFrame();
    }
    vkDeviceWaitIdle(app.device);
}


//PUBLIC
int ThING::API::getCircleAmount(){
    return app.circleCenters.size();
}

void ThING::API::getWindowSize(int* x, int* y){
    glfwGetWindowSize(app.windowManager.getWindow(), x, y);
    return;
}

void ThING::API::addCircle(std::string id, glm::vec2 pos, float size, glm::vec3 color){
    app.circleCenters.push_back({pos, size, color});
    std::string uniqueId = makeUniqueId(id);
    circleIds[uniqueId] = app.circleCenters.size() - 1;
    circleIdsByIndex.push_back(uniqueId);
}

Circle& ThING::API::getCircle(const std::string& id){
    int i = 0;
    if(circleIds.contains(id)){
        return app.circleCenters[circleIds[id]];
    }
    static Circle nullCircle = NULL_CIRCLE;
    return nullCircle;
}

bool ThING::API::deleteCircle(const std::string& id){
    if(!circleIds.contains(id)){
        return false;
    }
    if(circleIds[id] == app.circleCenters.size() - 1){
        app.circleCenters.pop_back();
        circleIdsByIndex.pop_back();
        circleIds.erase(id);
        return true;
    }
    app.circleCenters[circleIds[id]] = std::move(app.circleCenters.back());
    app.circleCenters.pop_back();
    circleIdsByIndex[circleIds[id]] = std::move(circleIdsByIndex.back());
    circleIdsByIndex.pop_back();
    circleIds[circleIdsByIndex[circleIds[id]]] = circleIds[id];
    circleIds.erase(id);
    return true;
}

std::vector<Circle>* ThING::API::getCircleDrawVector(){
    return &app.circleCenters;
}

void ThING::API::setZoomAndOffset(float zoom, glm::vec2 offset){
    app.zoom = zoom;
    app.offset = offset;
}

void ThING::API::setBackgroundColor(glm::vec4 color){
    app.clearColor.color.float32[0] = color.x;
    app.clearColor.color.float32[1] = color.y;
    app.clearColor.color.float32[2] = color.z;
    app.clearColor.color.float32[3] = color.w;
}

glm::mat4 ThING::API::build2DTransform(glm::vec2 pos, float rotation, glm::vec2 scale) {
    glm::mat4 t(1.0f);
    t = glm::translate(t, glm::vec3(pos, 0.0f));
    t = glm::rotate(t, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    t = glm::scale(t, glm::vec3(scale, 1.0f));
    return t;
}


void ThING::API::setPolygonTransform(std::string id, glm::vec2 pos, float rotation, glm::vec2 scale){
    for(Polygon& pol : app.polygons){
        if(pol.id == id){
            pol.transform = {pos, rotation, scale};
        }
    }
}

std::string ThING::API::makeUniqueId(const std::string& baseId) {
    std::string uniqueId = baseId;
    int counter = 1;

    bool exists = true;
    while (exists) {
        exists = false;
        if(polygonIds.contains(uniqueId)){
            exists = true;
            uniqueId = baseId + std::to_string(counter++);
        }
        if(exists){
            continue;
        }
        if(circleIds.contains(uniqueId)){
            exists = true;
            uniqueId = baseId + std::to_string(counter++);
        }
    }
    return uniqueId;
}


void ThING::API::addPolygon(std::string id, glm::vec2 pos, float rotation, glm::vec2 scale, std::vector<Vertex>& ver, std::vector<uint16_t>& ind){
    app.polygons.push_back({makeUniqueId(id), 
        static_cast<uint32_t>(app.vertices.size()), 
        static_cast<uint32_t>(ver.size()),
        static_cast<uint32_t>(app.indices.size()),
        static_cast<uint32_t>(ind.size()),
        {pos, rotation, scale}
    });
    app.vertices.reserve(app.vertices.size() + ver.size());
    app.indices.reserve(app.indices.size() + ind.size());
    app.vertices.insert(app.vertices.end(), ver.begin(), ver.end());
    app.indices.insert(app.indices.end(), ind.begin(), ind.end());
    polygonIds[app.polygons.back().id] = app.polygons.size() - 1;
}

void ThING::API::addPolygon(std::string id, glm::vec2 pos, float rotation, glm::vec2 scale, float windingSign, std::vector<Vertex>&& ver, std::vector<uint16_t>&& ind){
    app.polygons.push_back({makeUniqueId(id), 
        static_cast<uint32_t>(app.vertices.size()), 
        static_cast<uint32_t>(ver.size()),
        static_cast<uint32_t>(app.indices.size()),
        static_cast<uint32_t>(ind.size()),
        {pos, rotation, scale, 5.f, {1, 1, 0, .1}, windingSign}
    });
    app.vertices.reserve(app.vertices.size() + ver.size());
    app.indices.reserve(app.indices.size() + ind.size());
    app.vertices.insert(app.vertices.end(), std::make_move_iterator(ver.begin()), std::make_move_iterator(ver.end()));
    app.indices.insert(app.indices.end(), std::make_move_iterator(ind.begin()), std::make_move_iterator(ind.end()));
    polygonIds[app.polygons.back().id] = app.polygons.size() - 1;
    std::cout << app.vertices.size() << std::endl;
    std::cout << app.indices.size() << std::endl;
}

void ThING::API::addPolygon(Polygon&& polygon, std::vector<Vertex>&& ver, std::vector<uint16_t>&& ind){
    app.polygons.push_back({polygon.id, 
        static_cast<uint32_t>(app.vertices.size()), 
        static_cast<uint32_t>(ver.size()),
        static_cast<uint32_t>(app.indices.size()),
        static_cast<uint32_t>(ind.size()),
        {polygon.transform.position, polygon.transform.rotation, polygon.transform.scale}
    });
    app.vertices.insert(app.vertices.end(), std::make_move_iterator(ver.begin()), std::make_move_iterator(ver.end()));
    app.indices.insert(app.indices.end(), std::make_move_iterator(ind.begin()), std::make_move_iterator(ind.end()));
    polygonIds[app.polygons.back().id] = app.polygons.size() - 1;
}

Polygon& ThING::API::getPolygon(const std::string& id){
    if(polygonIds.contains(id)){
        Polygon& pol = app.polygons[polygonIds[id]];
        if(pol.id == id)
            if(pol.alive)
                return pol;
    }
    static Polygon nullPolygon{};
    return nullPolygon;
}

bool ThING::API::deletePolygon(const std::string& id){
    if(!polygonIds.contains(id)){
        return false;
    }
    app.polygons[polygonIds[id]].alive = false;
    static int deathPolygons = 0;
    deathPolygons++;
    
    if((float)deathPolygons/app.polygons.size() > .25f){
        deathPolygons = 0;
        cleanPolygonsList();
    }
    return true;
}

void ThING::API::cleanPolygonsList() {
    size_t aliveCount = 0;
    size_t reserveVerticesSize = 0, reserveIndicesSize = 0;
    
    for (size_t i = 0; i < app.polygons.size(); i++) {
        const Polygon& pol = app.polygons[i];
        if (!pol.alive) 
            continue;
        aliveCount++;
        reserveVerticesSize += pol.vertexCount;
        reserveIndicesSize += pol.indexCount;
    }
    if (aliveCount == 0) {
        app.polygons.clear();
        app.vertices.clear();
        app.indices.clear();
        polygonIds.clear();
        return;
    }
    std::vector<Polygon> oldPolygons = std::move(app.polygons);
    std::vector<uint16_t> oldIndices = std::move(app.indices);
    std::vector<Vertex> oldVertices = std::move(app.vertices);
    app.polygons.clear();
    app.polygons.reserve(aliveCount);
    app.indices.clear();
    app.indices.reserve(reserveIndicesSize);
    app.vertices.clear();
    app.vertices.reserve(reserveVerticesSize);
    for(Polygon& pol : oldPolygons){
        if(!pol.alive)
            continue;
        std::vector<Vertex> ver(oldVertices.begin() + pol.vertexOffset, oldVertices.begin() + pol.vertexOffset + pol.vertexCount);
        std::vector<uint16_t> ind(oldIndices.begin() + pol.indexOffset, oldIndices.begin() + pol.indexOffset + pol.indexCount);
        addPolygon(std::move(pol), std::move(ver), std::move(ind));
    }
}

bool ThING::API::exists(const Polygon& polygon){
    if(polygon.alive)
        return true;
    return false;
}

bool ThING::API::exists(const Circle& circle){
    if(circle != NULL_CIRCLE)
        return true;
    return false;
}
static float polygonWindingSign(const std::vector<Vertex>& verts) {
    if (verts.size() < 3) return 1.0f;
    long double a = 0.0L;
    for (size_t i = 0, j = verts.size() - 1; i < verts.size(); j = i++) {
        a += (long double)verts[j].pos.x * (long double)verts[i].pos.y
           - (long double)verts[i].pos.x * (long double)verts[j].pos.y;
    }
    return (a >= 0.0L) ? 1.0f : -1.0f;
}
bool ThING::API::addRegularPol(std::string id, size_t sides, glm::vec2 pos, glm::vec2 scale, glm::vec3 color){
    if(sides < 3){
        return false;
    }
    std::vector<Vertex> vertices;
    vertices.reserve(sides);
    for(int i = 0; i < sides; i++){
        glm::vec2 vtx = {sin((6.28 * i / (float)sides)), cos((6.28 * i / (float)sides))};
        vertices.push_back({vtx, {glm::normalize(vtx)}});
        vertices[i].color = color;
    }
    vertices[0].prev = vertices[sides - 1].pos;
    vertices[0].next = vertices[1].pos;
    for(int i = 1; i < sides - 1; i++){
        vertices[i].prev = vertices[i - 1].pos;
        vertices[i].next = vertices[i + 1].pos;
    }
    vertices[sides - 1].prev = vertices[sides - 2].pos;
    vertices[sides - 1].next = vertices[0].pos;
    std::vector<uint16_t> indices;
    indices.reserve((sides - 2) * 3);
    for(int i = 0; i < sides - 2;){
        indices.push_back(0);
        indices.push_back(++i);
        indices.push_back(++i);
        i--;
    }
    float wSign = polygonWindingSign(vertices);
    float scaleSign = (scale.x * scale.y >= 0.0f) ? 1.0f : -1.0f;
    float windingSign = wSign * scaleSign;
    addPolygon(id, pos, 0.f, scale, windingSign, std::move(vertices), std::move(indices));
    return true;
}

std::span<const std::string_view> ThING::API::viewPolygonIdList(){
    static std::vector<std::string_view> idList;
    idList.clear();
    idList.reserve(app.polygons.size());
    for(const Polygon& pol : app.polygons){
        if(pol.alive)
            idList.emplace_back(pol.id);
    }
    return idList;
}

bool ThING::API::playAudio(const std::string& soundFile){
    if(ma_engine_play_sound(&audioEngine, soundFile.c_str(), nullptr) != MA_SUCCESS){
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

ThING::Collision ThING::API::getCircleCollision(const Circle& circle1, const Circle& circle2){
    ThING::Collision hit;
    const glm::vec2 collision = circle2.pos - circle1.pos;
    const float summedSize = circle1.size + circle2.size;
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