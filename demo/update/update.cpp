#include "update.h"
#include <ThING/core.h>
#include <ThING/extras/handMade.h>
#include "ThING/api.h"
#include "ThING/types/enums.h"
#include "ThING/types/renderData.h"
#include "glm/fwd.hpp"
#include "globals.h"
#include "imgui.h"
#include "physicsObject.h"
#include <cstdint>
#include <span>

void update(ThING::API& api, FPSCounter& fps){
    std::span<InstanceData> circleInstances = api.getInstanceVector(InstanceType::Circle);

    WindowSize windowSize;
    api.getWindowSize(&windowSize.width, &windowSize.height);
    windowSize.width /= 2;
    windowSize.height /= 2;

    static uint32_t count = 0;
    static std::vector<PhysicsObject> circlePhysics = {};
    static std::vector<Entity> lineEntities;

    const float BIGGER_RADIUS = 4;
    const float BIGGER_RADIUS_MINUS = BIGGER_RADIUS - 1;
    const float SMALLER_RADIUS = 2;

    // ===== CLICK EVENTS START =====
    ImGuiIO& io = ImGui::GetIO();

    if (!io.WantCaptureMouse){
        if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
            ImVec2 tempPosition = ImGui::GetMousePos();
            glm::vec2 pos = {
                tempPosition.x - windowSize.width,
                tempPosition.y - windowSize.height
            };

            if(pos.x > -windowSize.width + dockedSizeX + 5){

                float circleSize = getRandomNumber(SMALLER_RADIUS, BIGGER_RADIUS);
                count++;
                // glm::vec4 color = {getRandomNumber(0.0f, 1.0f), getRandomNumber(0.0f, 1.0f), getRandomNumber(0.0f, 1.0f), 1};

                Entity e = api.addCircle(pos, circleSize, {0,0,1,1.f});
                circleInstances = api.getInstanceVector(InstanceType::Circle);        

                api.getInstance(e).outlineSize = 5;
                api.getInstance(e).outlineColor = {0, 0, 1, .4f};
                
                if (e.index < circlePhysics.size()) {
                    circlePhysics[e.index] = {pos, pos, {0.f, 0.f}};
                } else {
                    circlePhysics.push_back({pos, pos, {0.f, 0.f}});
                }
            }
        }

        if(ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseDragging(ImGuiMouseButton_Right)){
            ImVec2 tempPosition = ImGui::GetMousePos();
            glm::vec2 p = {
                tempPosition.x - windowSize.width,
                tempPosition.y - windowSize.height
            };

            Entity hit = INVALID_ENTITY;

            for (uint32_t i = 0; i < circleInstances.size(); i++) {
                const InstanceData& c = circleInstances[i];

                if(!c.alive) continue;

                glm::vec2 d = p - c.position;
                if (glm::dot(d, d) <= c.scale.x * c.scale.x) {
                    hit = {i, InstanceType::Circle};
                    break;
                }
            }

            if (api.exists(hit)) {
                circlePhysics[hit.index] = {};

                api.deleteInstance(hit);
            }
        }
        // ===== CLICK EVENTS ENDS =====
    }

    const int steps = 4;
    const float sub_dt = simSpeed / (float)steps;

    int circleAmount = circleInstances.size();

    int gridWidth  = (windowSize.width  + BIGGER_RADIUS_MINUS) / BIGGER_RADIUS;
    int gridHeight = (windowSize.height + BIGGER_RADIUS_MINUS) / BIGGER_RADIUS;

    static std::vector<std::vector<uint32_t>> circleID(gridWidth * gridHeight);
    static WindowSize lastWindowSize = windowSize;
    static bool firstTime = true;
    if(lastWindowSize != windowSize){
        circleID.clear();
        circleID.resize(gridWidth * gridHeight);
        firstTime = true;
        lastWindowSize = windowSize;
    }
    if (firstTime) {
        for (auto& cell : circleID) {
            cell.reserve(8);
        }
        firstTime = false;
    }

    for (auto& cell : circleID){
        cell.clear();
    }

    for (int i = 0; i < circleAmount; i++){
        if(!circleInstances[i].alive) 
            continue;

        int gridX = ((circleInstances[i].position.x + windowSize.width)  / 2) / BIGGER_RADIUS;
        int gridY = ((circleInstances[i].position.y + windowSize.height) / 2) / BIGGER_RADIUS;

        if (gridX >= 0 && gridX < gridWidth && gridY >= 0 && gridY < gridHeight)
        {
            circleID[(gridY * gridWidth) + gridX].push_back(i);
        }
    }

    for (int s = steps; s; s--){

        for(uint32_t i = 0; i < circlePhysics.size(); i++){
            if(!circleInstances[i].alive) 
                continue;
            circlePhysics[i].accelerate({gravity[0], gravity[1]});
        }

        collissionCount = 0;
        for(int i = 0; i < circleAmount; i++){
            if(!circleInstances[i].alive) 
                continue;

            circlePhysics[i].updatePos(sub_dt);
            circleInstances[i].position = circlePhysics[i].currentPos;
        }

        for (int x = 0; x < gridWidth; x++) {
            for (int y = 0; y < gridHeight; y++) {
                for (uint32_t ID : circleID[(y * gridWidth) + x]) {
                    for (int j = 0; j <= 1; j++) {
                        for (int k = -1; k <= 1; k++) {

                            if (j == 0 && k == 1) continue;

                            int nx = x + j;
                            int ny = y + k;

                            if (nx < 0 || ny < 0 || nx >= gridWidth || ny >= gridHeight)
                                continue;

                            for (uint32_t neighbor : circleID[(ny * gridWidth) + nx]) {
                                if (ID == neighbor) continue;
                                if (j == 0 && k == 0 && ID > neighbor) continue;

                                Collision collision = getCircleCollision(circleInstances[ID], circleInstances[neighbor]);

                                if(collision.hit){
                                    collissionCount++;
                                    circlePhysics[ID].currentPos -= collision.normal * collision.depth * stiffness;
                                    circlePhysics[neighbor].currentPos += collision.normal * collision.depth * stiffness;
                                }
                            }
                        }
                    }
                }
            }
        }

        for(int i = 0; i < circleAmount; i++){
            if(!circleInstances[i].alive) 
                continue;

            const glm::vec2 minBound{-windowSize.width + dockedSizeX, -windowSize.height};
            const glm::vec2 maxBound{ windowSize.width, windowSize.height};

            float r = circleInstances[i].scale.x;

            if (circlePhysics[i].currentPos.x < minBound.x + r)
                circlePhysics[i].currentPos.x = minBound.x + r;

            if (circlePhysics[i].currentPos.x > maxBound.x - r)
                circlePhysics[i].currentPos.x = maxBound.x - r;

            if (circlePhysics[i].currentPos.y < minBound.y + r)
                circlePhysics[i].currentPos.y = minBound.y + r;

            if (circlePhysics[i].currentPos.y > maxBound.y - r)
                circlePhysics[i].currentPos.y = maxBound.y - r;
        }
    }
    static std::vector<uint32_t> aliveCircles;
    aliveCircles.clear();

    for (uint32_t i = 0; i < circleInstances.size(); i++){
        if(circleInstances[i].alive){
            aliveCircles.push_back(i);
        }
    }

    uint32_t neededLines = aliveCircles.size() > 1 ? aliveCircles.size() - 1 : 0;

    while (lineEntities.size() < neededLines) {
        Entity e = api.addLine({0,0}, {0,0}, 2.0f);
        lineEntities.push_back(e);
    }

    while (lineEntities.size() > neededLines) {
        api.deleteInstance(lineEntities.back());
        lineEntities.pop_back();
    }

    for (uint32_t i = 0; i < neededLines; i++) {
        uint32_t a = aliveCircles[i];
        uint32_t b = aliveCircles[i + 1];

        Entity line = lineEntities[i];
        LineData& ld = api.getLineVector()[line.index];

        ld.point1 = circleInstances[a].position;
        ld.point2 = circleInstances[b].position;
    }
}
