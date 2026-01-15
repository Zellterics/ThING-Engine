#include "ui.h"
#include "glm/fwd.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include "globals.h"
#include <ThING/extras/imGuiCustom.h>
#include <cstdint>
#include <string>
#include <unordered_map>

void UI(ThING::API& api, FPSCounter& fps){
    static glm::vec2 offset = {0,0};
    static float zoom = 1;

    static float bgColor[3] = {0,0,0};
    static glm::vec4 bgColorVec = {0,0,0,1};
    static std::unordered_map<Entity, uint8_t> openedWindows;

    ImGui::GetIO().IniFilename = nullptr;
    ImGui::SetNextWindowBgAlpha(0.f);

    ImGuiID dockspaceID = ImGui::DockSpaceOverViewport(
            0,
            ImGui::GetMainViewport(),
            ImGuiDockNodeFlags_PassthruCentralNode |
            (ImGuiDockNodeFlags)ImGuiDockNodeFlags_NoTabBar
        );

    static bool first = true;
    if(first){
        first = false;

        monocraft = ImGui::GetIO().Fonts->AddFontFromFileTTF("../external/Monocraft.ttf", 18.0f);
        IM_ASSERT(monocraft && "Failed to load font!");

        ImGui::DockBuilderRemoveNode(dockspaceID);
        ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_CentralNode);
        ImGui::DockBuilderSetNodeSize(dockspaceID, ImGui::GetMainViewport()->Size);

        ImGuiID dock_id_left = 0, dock_id_right = 0;
        ImGui::DockBuilderSplitNode( dockspaceID, ImGuiDir_Left, 0.25f, &dock_id_left, &dock_id_right);
        ImGui::DockBuilderDockWindow("Debug Window", dock_id_left);

        ImGui::DockBuilderFinish(dockspaceID);
    }

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowBorderSize = 0.0f;
    style.Colors[ImGuiCol_DockingEmptyBg] = {1,1,1,0};
    style.Colors[ImGuiCol_WindowBg] = {.01f,.01f,.01f,1};
    style.Colors[ImGuiCol_FrameBg] = {0,0,0,1};
    style.Colors[ImGuiCol_Border] = {1,1,1,0};
    style.WindowMinSize = ImVec2(200, 200);

    ImGui::Begin("Debug Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

    ImVec2 dockedSize = ImGui::GetWindowSize();

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.f);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 12.f);
    ImGui::PushItemWidth(100.f);
    ImGui::PushFont(monocraft);

    ImGui::Text("Circles: %u", api.getInstanceCount(InstanceType::Circle));
    ImGui::PushItemWidth(150.0f);
    ImGui::SliderInt("FPS", &speed, 10, 420);
    ImGui::PopItemWidth();
    Slider2DFloat("Gravity", &gravity[0], &gravity[1], -5, 5, -5, 5);
    if(gravity[0] < .2 && gravity[0] > -.2){
        gravity[0] = 0;
    }
    if(gravity[1] < .2 && gravity[1] > -.2){
        gravity[1] = 0;
    }
    ImGui::SliderFloat("Stiffness", &stiffness, 0.01f, 0.4f, "%.3f");

    ImGui::Text("Real FPS: %d", (int)(fps.getFPS() + 1));
    ImGui::Text("Collisions: %u", collissionCount);

    if(ImGui::Button("Random Polygon")){
        int sides = getRandomNumber(3, 12);
        float x = getRandomNumber(-200.f, 600.f);
        float y = getRandomNumber(-400.f, 400.f);

        api.addRegularPol(
            sides,
            {x,y},
            {40,40},
            {getRandomNumber(0.f,1.f), getRandomNumber(0.f,1.f), getRandomNumber(0.f,1.f), 1.f}
        );
    }

    if(ImGui::TreeNode("Polygons")){
        auto polys = api.getInstanceVector(InstanceType::Polygon);

        for(uint32_t i = 0; i < polys.size(); i++){
            if(!polys[i].alive) continue;

            Entity e{ i, InstanceType::Polygon };

            ImGui::Text("Polygon %u", i);

            if(ImGui::IsItemClicked(ImGuiMouseButton_Right)){
                openedWindows[e] = 1;
            }

            bool open = openedWindows[e] != 0;

            if(open){
                ImGui::Begin( ("Polygon " + std::to_string(i)).c_str(), &open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

                ImGui::PushID(i);

                ImGui::SliderFloat("Rotation", &polys[i].rotation, -10.f, 10.f);

                Slider2DFloat("Scale", &polys[i].scale.x, &polys[i].scale.y, -500, 500, -500, 500);
                Slider2DFloat("Position", &polys[i].position.x, &polys[i].position.y, -500, 500, -500, 500);

                if(ImGui::Button("Delete")){
                    api.deleteInstance(e);
                    open = false;
                }

                ImGui::PopID();
                ImGui::End();
            }
            openedWindows[e] = open ? 1 : 0;
        }

        ImGui::TreePop();
    }

    if(ImGui::TreeNode("Extras")){
        ImGui::SliderFloat("Zoom", &zoom, 0, 5);
        Slider2DFloat("Offset", &offset.x, &offset.y, -500, 500, -500, 500);
        ImGui::ColorPicker3("BG Color", bgColor);
        ImGui::TreePop();
    }

    ImGui::PopStyleVar(2);
    ImGui::PopItemWidth();
    ImGui::PopFont();
    ImGui::End();

    bgColorVec = {bgColor[0], bgColor[1], bgColor[2], 1};

    if(offset.x < 40 && offset.x > -40) offset.x = 0;
    if(offset.y < 40 && offset.y > -40) offset.y = 0;

    api.setBackgroundColor(bgColorVec);
    api.setZoom(zoom);
    api.setOffset(offset);

    ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockspaceID);
    dockedSizeX = dockedSize.x;

    if(node && node->CentralNode){
        simWidth  = (int)node->CentralNode->Size.x;
        simHeight = (int)node->CentralNode->Size.y;
    }

    fps.delay(1.f / (float)speed);
    fps.frame();
}
