#pragma once
#include <ThING/types/apiTypes.h>
#include <ThING/types/renderData.h>
#include "ThING/types/contexts.h"
#include "ThING/types/enums.h"
#include "glm/fwd.hpp"
#include <ThING/core.h>
#include <cstddef>
#include <cstdint>
#include <string>
#include <sys/types.h>
#include <vector>
#include <miniaudio.h>


const uint8_t ApiFlags_None = 0;
const uint8_t ApiFlags_UpdateCallbackFirst = 1 << 0;


namespace ThING{
    class API{
    public:
        API();
        API(uint8_t flags);
        ~API();

        bool setUpdateCallback(std::function<void(ThING::API&, FPSCounter&)>);
        bool setUICallback(std::function<void(ThING::API&, FPSCounter&)>);
        void run();

        uint32_t getInstanceCount(InstanceType type);
        void getWindowSize(int* x, int* y);
        Entity addCircle(glm::vec2 pos, float size, glm::vec4 color);
        std::span<InstanceData> getInstanceVector(InstanceType type);
        std::span<LineData> getLineVector();
        void setZoom(float zoom);
        void setOffset(glm::vec2 offset);
        void setBackgroundColor(glm::vec4 color);
        Entity addPolygon(glm::vec2 pos, glm::vec4 color, glm::vec2 scale, std::span<Vertex> ver, std::span<uint16_t> ind);
        Entity addPolygon(glm::vec2 pos, glm::vec4 color, glm::vec2 scale, std::vector<Vertex>&& ver, std::vector<uint16_t>&& ind);
        Entity addLine(glm::vec2 point1, glm::vec2 point2, float width);
        bool exists(const Entity e);
        bool deleteInstance(const Entity e);
        InstanceData& getInstance(const Entity e);
        LineData& getLine(const Entity e);
        Entity addRegularPol(size_t sides, glm::vec2 pos, glm::vec2 scale, glm::vec4 color);
        bool playAudio(const std::string& soundFile);
        bool playAudio(const std::string& soundFile, uint8_t volume);

        void updateOutlines() {dirtyFlags.ssbo = true;}
    private:
        // Entity addPolygon(InstanceData&& polygon, std::vector<Vertex>&& ver, std::vector<uint16_t>&& ind); add if needed
        Entity addCircle(InstanceData&& instance);
        Entity addLine(LineData&& line);
        // void cleanRenderData(); add if a lot of death objects exists, right now I don't plan to use it

        void mainLoop();

        std::vector<InstanceData> circleInstances;
        std::vector<LineData> lineInstances;
        std::vector<InstanceData> polygonInstances;
        std::vector<MeshData> polygonMeshes;

        std::vector<Entity> circleFreeList;
        std::vector<Entity> lineFreeList;
        std::vector<Entity> polygonFreeList;


        std::function<void(ThING::API&, FPSCounter&)> updateCallback;
        std::function<void(ThING::API&, FPSCounter&)> uiCallback;

        DirtyFlags dirtyFlags;

        ma_engine audioEngine;
        ProtoThiApp app;

        uint8_t apiFlags;
    };
}