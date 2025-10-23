#include "update.h"
#include <ThING/core.h>
#include <ThING/extras/handMade.h>
#include "ThING/types/collission.h"
#include "glm/common.hpp"
#include "glm/fwd.hpp"
#include "globals.h"
#include "imgui.h"
#include "physicsObject.h"
#include <cstdint>
#include <string>

void update(ThING::API& api, FPSCounter& fps){
    std::vector<Circle>* circleCenters = api.getCircleDrawVector();
    int x, y;
    static bool changedResolution = false;
    api.getWindowSize(&x, &y);
    x /= 2;
    y /= 2;
    static uint32_t count = 0;
    static std::vector<PhysicsObject> circlePhysics = {};
    glm::vec2 randomPos = {
        getRandomNumber((x - dockedSizeX / 2.f) * spawnPoint[0] + dockedSizeX / 2.f, ((x - dockedSizeX / 2.f - spawnRadius) * spawnPoint[0]) + spawnRadius + dockedSizeX / 2.f), 
        getRandomNumber(y * spawnPoint[1], ((y - spawnRadius) * spawnPoint[1]) + spawnRadius)};
    const float BIGGER_RADIUS = 8;
    const float BIGGER_RADIUS_MINUS = BIGGER_RADIUS - 1;
    const float SMALLER_RADIUS = 4;
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
        ImVec2 pos = ImGui::GetMousePos();
        glm::vec2 poss = {pos.x - (x), pos.y - (y)};
        if(poss.x > -x + dockedSizeX + 5){
            glm::vec3 color = {getRandomNumber(0.0f,100.0f)/100.0f, getRandomNumber(0.0f,100.0f)/100.0f, getRandomNumber(0.0f,100.0f)/100.0f};
            count++;
            api.addCircle(std::to_string(count), poss,getRandomNumber(SMALLER_RADIUS,BIGGER_RADIUS), color);
            api.getCircle(std::to_string(count)).outlineSize = .5;
            color = glm::abs(color - 1.f);
            api.getCircle(std::to_string(count)).outlineColor = {color, 1};
            circlePhysics.push_back({poss, poss, {0.f,0.f}});
        }
    }
    if(ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseDragging(ImGuiMouseButton_Right)){
        ImVec2 pos = ImGui::GetMousePos();
        glm::vec2 p = { pos.x - x, pos.y - y };

        int hit = -1;
        for (int i = 0; i < (int)circleCenters->size(); ++i) {
            const Circle& c = (*circleCenters)[i];
            glm::vec2 d = p - c.pos;
            float d2 = glm::dot(d, d);
            if (d2 <= c.size * c.size) { hit = i; break; }
        }
        if (hit != -1) {
            const std::string& delId = api.getCircleIdByIndex((std::size_t)hit);

            circlePhysics[hit] = std::move(circlePhysics.back());
            circlePhysics.pop_back();

            api.deleteCircle(delId);
        }
    }
    const int steps = 4;
    const float sub_dt = simSpeed / (float)steps;
    int circleAmount = api.getCircleAmount();
    int gridWidth = (x + BIGGER_RADIUS_MINUS) / BIGGER_RADIUS;
    int gridHeight = (y + BIGGER_RADIUS_MINUS) / BIGGER_RADIUS;
    static std::vector<std::vector<int>> circleID(gridWidth * gridHeight);
    // DEAR GOD I NEED TO CLEAN THIS CODE UP, BUT I NEED TO FINISH THIS SOON!!!!
    static int saveX = x, saveY = y;//YEP I NEED MORE DESCRIPTIVE NAMES
    static bool firstTime = true;
    if(saveX != x || saveY != y){
        changedResolution = true;
    }
    if(changedResolution){
        circleID.clear();
        circleID.resize(gridWidth * gridHeight);
        changedResolution = false;
        firstTime = true;
    }
    if (firstTime) {
        for (std::vector<int> &reserveVector : circleID) {
            reserveVector.reserve(8);
        }
        firstTime = false;
    }

    for (int i = steps; i; i--){
        for (std::vector<int> &clearID : circleID){
            clearID.clear();
        }
        for (int i = 0; i < circleAmount; i++){//OPTIMIZE THIS LATER
            int gridX = (((*circleCenters)[i].pos.x + x) / 2) / BIGGER_RADIUS;
            int gridY = (((*circleCenters)[i].pos.y + y) / 2) / BIGGER_RADIUS;
            int index = (gridY * gridWidth) + gridX;
            if (gridX >= 0 && gridX < gridWidth &&
                gridY >= 0 && gridY < gridHeight) {
                circleID[index].push_back(i);
            }
        }

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
                                ThING::Collision collision = api.get2ObjCollision(actualCircleCenter, neighborCircleCenter);
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
            const glm::vec2 minBound{-x + dockedSizeX, -y};
            const glm::vec2 maxBound{ x, y};

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