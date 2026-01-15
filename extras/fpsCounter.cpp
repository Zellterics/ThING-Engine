#include <ThING/extras/fpsCounter.h>
#include <iostream>

bool FPSCounter::frame() {
    using namespace std::chrono;

    auto now = steady_clock::now();
    deltaTime = duration_cast<duration<float>>(now - frameStart).count();
    elapsedTime += deltaTime;
    frameStart = now;
    frameCount++;

    if (elapsedTime >= 1.0f) {
        #ifdef DEBUG
        std::cout << "FPS: " << frameCount << std::endl;
        #endif
        frameCount = 0;
        elapsedTime = 0.0f;
        return true;
    }
    return false;
}

void FPSCounter::delay(float targetFrameTime) {
    using namespace std::chrono;

    auto now = steady_clock::now();
    float elapsed = duration_cast<duration<float>>(now - frameStart).count();
    float remaining = targetFrameTime - elapsed;

    if (remaining > 0.0f) {
        while (duration_cast<duration<float>>(steady_clock::now() - frameStart).count() < targetFrameTime) {}
    }
}

int FPSCounter::getFPS() const {
    return (deltaTime > 0.0f) ? static_cast<int>(1.0f / deltaTime) : 0;
}

float FPSCounter::getDeltaTime() const {
    return deltaTime;
}