#pragma once
#include <ThING/core.h>
#include <string_view>
#include <vector>

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
        void addCircle(glm::vec2 pos, float size, glm::vec3 color);
        std::vector<Circle>* getCircleDrawVector();
        void setZoomAndOffset(float zoom, glm::vec2 offset);
        void setBackgroundColor(glm::vec4 color);
        glm::mat4 build2DTransform(glm::vec2 pos, float rotation, glm::vec2 scale);
        void setPolygonTransform(std::string id, glm::vec2 pos, float rotation, glm::vec2 scale);
        std::string makeUniqueId(std::string& baseId);
        void addPolygon(std::string id, glm::vec2 pos, float rotation, glm::vec2 scale, std::vector<Vertex>& ver, std::vector<uint16_t>& ind);
        void addPolygon(std::string id, glm::vec2 pos, float rotation, glm::vec2 scale, std::vector<Vertex>&& ver, std::vector<uint16_t>&& ind);
        Polygon& getPolygon(std::string id);
        bool exists(Polygon& polygon);
        bool addRegularPol(std::string id, size_t sides, glm::vec2 pos, glm::vec2 scale, glm::vec3 color);
        std::span<std::string_view> viewPolygonIdList();
    private:
        void mainLoop();

        std::function<void(ThING::API&, FPSCounter&)> updateCallback;
        std::function<void(ThING::API&, FPSCounter&)> uiCallback;

        


        ProtoThiApp app;
    };
}