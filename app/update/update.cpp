#include "update.h"
#include <ThING/core.h>
#include <ThING/extras/handMade.h>
#include "ThING/consts.h"
#include "glm/fwd.hpp"
#include "globals.h"
#include "imgui.h"
#include "physicsObject.h"
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

void update(ThING::API& api, FPSCounter& fps){
    std::vector<Circle>* circleCenters = api.getCircleDrawVector();
    WindowSize windowSize;
    api.getWindowSize(&windowSize.width, &windowSize.height);
    windowSize.width /= 2;
    windowSize.height /= 2;
    static uint32_t count = 0;
    static std::vector<PhysicsObject> circlePhysics = {};
    const float BIGGER_RADIUS = 4;
    const float BIGGER_RADIUS_MINUS = BIGGER_RADIUS - 1;
    const float SMALLER_RADIUS = 2;
    // ===== CLICK EVENTS START =====
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
        ImVec2 tempPosition = ImGui::GetMousePos();
        glm::vec2 pos = {tempPosition.x - (windowSize.width), tempPosition.y - (windowSize.height)};
        if(pos.x > -windowSize.width + dockedSizeX + 5){
            glm::vec3 circleColor = {getRandomNumber(0.f,1.f), getRandomNumber(0.f,1.f), getRandomNumber(0.f,1.f)};
            float circleSize =getRandomNumber(SMALLER_RADIUS,BIGGER_RADIUS);
            count++;
            api.addCircle(std::to_string(count), pos, circleSize, {0,0,1});
            api.getCircle(std::to_string(count)).outlineSize = 5;
            api.getCircle(std::to_string(count)).outlineColor = {1,0, 0, 1};
            if(speed < 100){//Change later
                api.getCircle(std::to_string(count)).objectID = count;
            }
            circlePhysics.push_back({pos, pos, {0.f,0.f}});
        }
    }
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseDragging(ImGuiMouseButton_Right)){
        ImVec2 tempPosition = ImGui::GetMousePos();
        glm::vec2 p = { tempPosition.x - windowSize.width, tempPosition.y - windowSize.height };

        int hit = -1;
        for (int i = 0; i < (int)circleCenters->size(); ++i) {
            const Circle& c = (*circleCenters)[i];
            glm::vec2 d = p - c.pos;
            float d2 = glm::dot(d, d);
            if (d2 <= c.size * c.size) { hit = i; break; }
        }
        if (hit != -1) {
            const std::string& delId = api.getCircleIdByIndex((std::size_t)hit);
            if (delId != NULL_CIRCLE_ID){
                circlePhysics[hit] = std::move(circlePhysics.back());
                circlePhysics.pop_back();

                api.deleteCircle(delId);
            }
        }
    }
    // ===== CLICK EVENTS ENDS =====
    const int steps = 4;
    const float sub_dt = simSpeed / (float)steps;
    int circleAmount = api.getCircleAmount();
    int gridWidth = (windowSize.width + BIGGER_RADIUS_MINUS) / BIGGER_RADIUS;
    int gridHeight = (windowSize.height + BIGGER_RADIUS_MINUS) / BIGGER_RADIUS;
    static std::vector<std::vector<int>> circleID(gridWidth * gridHeight);
    static WindowSize lastWindowSize = windowSize;
    static bool firstTime = true;
    if(lastWindowSize != windowSize){
        circleID.clear();
        circleID.resize(gridWidth * gridHeight);
        firstTime = true;
        lastWindowSize = windowSize;
    }
    if (firstTime) {
        for (std::vector<int> &reserveVector : circleID) {
            reserveVector.reserve(8);
        }
        firstTime = false;
    }
    for (std::vector<int> &clearID : circleID){
        clearID.clear();
    }
    for (int i = 0; i < circleAmount; i++){//OPTIMIZE THIS LATER
        int gridX = (((*circleCenters)[i].pos.x + windowSize.width) / 2) / BIGGER_RADIUS;
        int gridY = (((*circleCenters)[i].pos.y + windowSize.height) / 2) / BIGGER_RADIUS;
        int index = (gridY * gridWidth) + gridX;
        if (gridX >= 0 && gridX < gridWidth &&
            gridY >= 0 && gridY < gridHeight) {
            circleID[index].push_back(i);
        }
    }
    for (int i = steps; i; i--){

        for(PhysicsObject& circle : circlePhysics){
            circle.accelerate({gravity[0], gravity[1]});
        }

        collissionCount = 0;
        for(int i = 0; i < circleAmount; i++){
            circlePhysics[i].updatePos(sub_dt);

            (*circleCenters)[i].pos = circlePhysics[i].currentPos;
        }

        for (int x = 0; x < gridWidth; x++) { // 20 veces extremadamente rapido
            for (int y = 0; y < gridHeight; y++) { // 20 veces extremadamente rapido
                for (int ID : circleID[(y * gridWidth) + x]) {     // cantidad de objetos
                    for (int j = 0; j <= 1; j++) {   //2 veces
                        for (int k = -1; k <= 1; k++) { // 3 veces
                            if (j == 0 && k == 1)
                                continue;
                            int nx = x + j;
                            int ny = y + k;

                            if (nx < 0 || ny < 0 || nx >= gridWidth || ny >= gridHeight)
                                continue;

                            for (int neighbor : circleID[(ny * gridWidth) + nx]) {
                                if (ID == neighbor) continue;
                                if (j == 0 && k == 0 && ID > neighbor) continue;

                                PhysicsObject &neighborCirclePhysics = circlePhysics[neighbor];
                                Circle &neighborCircleCenter = (*circleCenters)[neighbor];
                                Circle &actualCircleCenter = (*circleCenters)[ID];
                                PhysicsObject &actualCirclePhysics = circlePhysics[ID];
                                ThING::Collision collision = api.getCircleCollision(actualCircleCenter, neighborCircleCenter);
                                if(collision.hit){
                                    collissionCount++;
                                    actualCirclePhysics.currentPos -= collision.normal * collision.depth * stiffness;
                                    neighborCirclePhysics.currentPos += collision.normal * collision.depth * stiffness;
                                }
                            }
                        }
                    }
                }
            }
        }
        
        for(int i = 0; i < circleAmount; i++){
            const glm::vec2 minBound{-windowSize.width + dockedSizeX, -windowSize.height};
            const glm::vec2 maxBound{ windowSize.width, windowSize.height};

            if (circlePhysics[i].currentPos.x < minBound.x + (*circleCenters)[i].size){ 
                circlePhysics[i].currentPos.x = minBound.x + (*circleCenters)[i].size;
            }
            if (circlePhysics[i].currentPos.x > maxBound.x - (*circleCenters)[i].size) {
                circlePhysics[i].currentPos.x = maxBound.x - (*circleCenters)[i].size;
            }
            if (circlePhysics[i].currentPos.y < minBound.y + (*circleCenters)[i].size) {
                circlePhysics[i].currentPos.y = minBound.y + (*circleCenters)[i].size;
            }
            if (circlePhysics[i].currentPos.y > maxBound.y - (*circleCenters)[i].size) {
                circlePhysics[i].currentPos.y = maxBound.y - (*circleCenters)[i].size;
            }
        }
    }
}