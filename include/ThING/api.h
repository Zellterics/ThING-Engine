#pragma once
#include "ThING/types/circle.h"
#include "ThING/types/polygon.h"
#include "glm/fwd.hpp"
#include <ThING/core.h>
#include <cstdint>
#include <string>
#include <string_view>
#include <sys/types.h>
#include <unordered_map>
#include <vector>
#include <miniaudio.h>
#include <ThING/types/collission.h>

namespace ThING{
    class API{
    public:
        API();
        ~API();

        bool setUpdateCallback(std::function<void(ThING::API&, FPSCounter&)>);
        bool setUICallback(std::function<void(ThING::API&, FPSCounter&)>);
        void run();

        int getCircleAmount();
        void getWindowSize(int* x, int* y);
        void addCircle(std::string id, glm::vec2 pos, float size, glm::vec3 color);
        Circle& getCircle(const std::string& id);
        bool deleteCircle(const std::string& id);
        std::vector<Circle>* getCircleDrawVector();
        void setZoomAndOffset(float zoom, glm::vec2 offset);
        void setBackgroundColor(glm::vec4 color);
        glm::mat4 build2DTransform(glm::vec2 pos, float rotation, glm::vec2 scale);
        void setPolygonTransform(std::string id, glm::vec2 pos, float rotation, glm::vec2 scale);
        std::string makeUniqueId(const std::string& baseId);
        void addPolygon(std::string id, glm::vec2 pos, float rotation, glm::vec2 scale, std::vector<Vertex>& ver, std::vector<uint16_t>& ind);
        void addPolygon(std::string id, glm::vec2 pos, float rotation, glm::vec2 scale, float windingSign, std::vector<Vertex>&& ver, std::vector<uint16_t>&& ind);
        bool deletePolygon(const std::string& id);
        void cleanPolygonsList();
        Polygon& getPolygon(const std::string& id);
        bool exists(const Polygon& polygon);
        bool exists(const Circle& circle);
        bool addRegularPol(std::string id, size_t sides, glm::vec2 pos, glm::vec2 scale, glm::vec3 color);
        /// @brief this returns a view, NEVER save this value
        std::span<const std::string_view> viewPolygonIdList();
        bool playAudio(const std::string& soundFile);
        bool playAudio(const std::string& soundFile, uint8_t volume);

        ThING::Collision get2ObjCollision(const Circle& circle1, const Circle& circle2);
        ThING::Collision get2ObjCollision(const Circle& circle, const Polygon& polygon);
        ThING::Collision get2ObjCollision(const Polygon& polygon, const Circle& circle);
        ThING::Collision get2ObjCollision(const Polygon& polygon1, const Polygon& polygon2);
        // MAYBE DELETE LATER ONLY TESTS
        std::string getCircleIdByIndex(std::size_t hit){return circleIdsByIndex[hit];}
    private:
        void addPolygon(Polygon&& polygon, std::vector<Vertex>&& ver, std::vector<uint16_t>&& ind);

        void mainLoop();

        std::function<void(ThING::API&, FPSCounter&)> updateCallback;
        std::function<void(ThING::API&, FPSCounter&)> uiCallback;
        
        std::unordered_map<std::string, uint32_t> circleIds;
        std::unordered_map<std::string, uint32_t> polygonIds;

        std::vector<std::string> circleIdsByIndex;

        ma_engine audioEngine;
        ProtoThiApp app;
    };
}