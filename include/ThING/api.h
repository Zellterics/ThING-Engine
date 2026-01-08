#pragma once
#include <ThING/types/apiTypes.h>
#include <ThING/types/renderData.h>
#include "ThING/types/enums.h"
#include "glm/fwd.hpp"
#include <ThING/core.h>
#include <cstddef>
#include <cstdint>
#include <string>
#include <sys/types.h>
#include <vector>
#include <miniaudio.h>

namespace ThING{
    class API{
    public:
        API();
        ~API();

        bool setUpdateCallback(std::function<void(ThING::API&, FPSCounter&)>);
        bool setUICallback(std::function<void(ThING::API&, FPSCounter&)>);
        void run();

        uint32_t getInstanceCount(InstanceType type);
        void getWindowSize(int* x, int* y);
        Entity addCircle(glm::vec2 pos, float size, glm::vec4 color);
        // Circle& getCircle(const std::string& id);
        // bool deleteCircle(const std::string& id);
        // std::vector<Circle>* getCircleDrawVector();
        std::span<InstanceData> getInstanceVector(InstanceType type);
        // void setZoomAndOffset(float zoom, glm::vec2 offset);
        void setZoom(float zoom);
        void setOffset(glm::vec2 offset);
        void setBackgroundColor(glm::vec4 color);
        // glm::mat4 build2DTransform(glm::vec2 pos, float rotation, glm::vec2 scale);
        // void setPolygonTransform(std::string id, glm::vec2 pos, float rotation, glm::vec2 scale);
        // std::string makeUniqueId(const std::string& baseId);
        Entity addPolygon(glm::vec2 pos, glm::vec4 color, glm::vec2 scale, std::vector<Vertex>& ver, std::vector<uint16_t>& ind);
        Entity addPolygon(glm::vec2 pos, glm::vec4 color, glm::vec2 scale, std::vector<Vertex>&& ver, std::vector<uint16_t>&& ind);
        bool exists(const Entity e);
        bool deleteInstance(const Entity e);
        InstanceData& getInstance(const Entity e);
        Entity addRegularPol(size_t sides, glm::vec2 pos, glm::vec2 scale, glm::vec4 color);
        /// @brief this returns a view, NEVER save this value
        // std::span<const std::string_view> viewPolygonIdList();
        bool playAudio(const std::string& soundFile);
        bool playAudio(const std::string& soundFile, uint8_t volume);

        // ThING::Collision getCircleCollision(const Circle& circle1, const Circle& circle2);
        // ThING::Collision get2ObjCollision(const Circle& circle, const Polygon& polygon);
        // ThING::Collision get2ObjCollision(const Polygon& polygon, const Circle& circle); Maybe, just maybe add later
        // ThING::Collision get2ObjCollision(const Polygon& polygon1, const Polygon& polygon2); I really think this is getting out of scope
        // std::string getCircleIdByIndex(std::size_t index){return circleIdsByIndex.size() > index ? circleIdsByIndex[index] : NULL_CIRCLE_ID;}
        // std::string getPolygonIdByIndex(std::size_t index){return app.polygons.size() > index ? app.polygons[index].id : NULL_POLYGON_ID;};
    private:
        Entity addPolygon(InstanceData&& polygon, std::vector<Vertex>&& ver, std::vector<uint16_t>&& ind);
        Entity addCircle(InstanceData&& instance);
        void cleanRenderData();

        void mainLoop();

        std::vector<InstanceData> circleInstances;
        std::vector<InstanceData> polygonInstances;
        std::vector<MeshData> polygonMeshes;

        std::vector<Entity> circleFreeList;
        std::vector<Entity> polygonFreeList;

        std::function<void(ThING::API&, FPSCounter&)> updateCallback;
        std::function<void(ThING::API&, FPSCounter&)> uiCallback;
        
        // std::unordered_map<std::string, uint32_t> circleIds;
        // std::unordered_map<std::string, uint32_t> polygonIds; Erase later when everythings over, so over...

        // std::vector<std::string> circleIdsByIndex;

        ma_engine audioEngine;
        ProtoThiApp app;
    };
}