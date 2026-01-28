#include <chrono>

class FPSCounter {
public:
    void beginFrame();
    void endFrame();
    void setTarget(float target);

    float getDeltaTime() const;
    int getFPS() const;
    int getInstantFPS() const;

private:
    float targetFPS = 60;
    std::chrono::steady_clock::time_point frameStart;
    float deltaTime = 0.0f;
    float fpsTimer = 0.0f;
    int frameCount = 0;
    int currentFPS = 0;
};