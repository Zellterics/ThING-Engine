#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include <ThING/types/vertex.h>
#include <cstdint>
#include <iterator>
#include <iostream>
#include <string>
#include <vector>
#include <ThING/api.h>
#include "imgui.h"

// Backends (GLFW + Vulkan)
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

//CONSTRUCTOR
ThING::API::API() : app(){
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

void ThING::API::addCircle(glm::vec2 pos, float size, glm::vec3 color){
    app.circleCenters.push_back({pos, size, color});
}

std::vector<Circle>* ThING::API::getCircleDrawVector(){
    return &app.circleCenters;
}

void ThING::API::setZoomAndOffset(float zoom, glm::vec2 offset){
    app.zoom = zoom;
    app.offset = offset;
    app.bufferManager.setUpdateUBOFlag(true);
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


void ThING::API::setRotation(glm::vec2 pos, float rotation, glm::vec2 scale){
    app.polygons[0].transform = {pos, rotation, glm::u8vec4{10, 255, 50, 50}, scale};
}

std::string ThING::API::makeUniqueId(std::string baseId) {
    std::string uniqueId = baseId;
    int counter = 1;

    bool exists = true;
    while (exists) {
        exists = false;
        for (const Polygon& pol : app.polygons) {
            if (pol.id == uniqueId) {
                exists = true;
                uniqueId = baseId + std::to_string(counter++);
                break;
            }
        }
    }
    return uniqueId;
}


void ThING::API::addPolygon(std::string& id, glm::vec2 pos, float rotation, glm::vec2 scale, std::vector<Vertex>& ver, std::vector<uint16_t>& ind){
    app.polygons.push_back({makeUniqueId(id), 
        static_cast<uint32_t>(app.vertices.size()), 
        static_cast<uint32_t>(ver.size()),
        static_cast<uint32_t>(app.indices.size()),
        static_cast<uint32_t>(ind.size()),
        true,
        {pos, rotation, glm::u8vec4{10, 255, 50, 50}, scale}
    });
    app.vertices.reserve(app.vertices.size() + ver.size());
    app.indices.reserve(app.indices.size() + app.indices.size());
    app.vertices.insert(app.vertices.end(), ver.begin(), ver.end());
    app.indices.insert(app.indices.end(), ind.begin(), ind.end());
}

void ThING::API::addPolygon(std::string& id, glm::vec2 pos, float rotation, glm::vec2 scale, std::vector<Vertex>&& ver, std::vector<uint16_t>&& ind){
    app.polygons.push_back({makeUniqueId(id), 
        static_cast<uint32_t>(app.vertices.size()), 
        static_cast<uint32_t>(ver.size()),
        static_cast<uint32_t>(app.indices.size()),
        static_cast<uint32_t>(ind.size()),
        true,
        {pos, rotation, glm::u8vec4{10, 255, 50, 50}, scale}
    });
    app.vertices.reserve(app.vertices.size() + ver.size());
    app.indices.reserve(app.indices.size() + ind.size());
    app.vertices.insert(app.vertices.end(), std::make_move_iterator(ver.begin()), std::make_move_iterator(ver.end()));
    app.indices.insert(app.indices.end(), std::make_move_iterator(ind.begin()), std::make_move_iterator(ind.end()));
    std::cout << app.vertices.size() << std::endl;
    std::cout << app.indices.size() << std::endl;
}

Polygon& ThING::API::getPolygon(std::string id){
    for(Polygon& pol : app.polygons)
        if(pol.id == id)
            return pol;
    app.polygons.emplace_back();
    return app.polygons.back();
}

bool ThING::API::exists(Polygon& polygon){
    if(polygon.alive)
        return true;
    return false;
}

bool ThING::API::addRegularPol(std::string id, size_t sides, glm::vec2 pos, glm::vec2 scale, glm::vec3 color){
    if(sides < 3){
        return false;
    }
    std::vector<Vertex> vertices;
    vertices.reserve(sides);
    for(int i = 0; i < sides; i++){
        vertices.push_back({{sin((6.28 * i / (float)sides)), cos((6.28 * i / (float)sides))}, color});
    }
    std::vector<uint16_t> indices;
    indices.reserve(sides);
    for(int i = 0; i < sides - 1;){
        indices.push_back(0);
        indices.push_back(++i);
        indices.push_back(++i);
        i--;
    }
    addPolygon(id, pos, 0.f, scale, std::move(vertices), std::move(indices));
    return true;
}