#include <ThING/extras/fpsCounter.h>
#include <chrono>
#include <thread>
#ifdef DEBUG
    #include <iostream>
#endif

void FPSCounter::beginFrame() {
    frameStart = std::chrono::steady_clock::now();
}

void FPSCounter::endFrame() {
    using namespace std::chrono;

    auto frameEnd = steady_clock::now();
    deltaTime = duration<float>(frameEnd - frameStart).count();

    float targetFrameTime = 1.0f / (targetFPS - 1);

    if (deltaTime < targetFrameTime) {
        std::this_thread::sleep_for(
            duration<float>(targetFrameTime - deltaTime)
        );

        frameEnd = steady_clock::now();
        deltaTime = duration<float>(frameEnd - frameStart).count();
    }

    fpsTimer += deltaTime;
    frameCount++;

    if (fpsTimer >= 1.0f) {
        currentFPS = frameCount;
        frameCount = 0;
        fpsTimer = 0.0f;

        #ifdef DEBUG
            std::cout << "FPS: " << currentFPS << std::endl;
        #endif
    }
}

void FPSCounter::setTarget(float target){
    targetFPS = target;
}

float FPSCounter::getDeltaTime() const {
    return deltaTime;
}

int FPSCounter::getFPS() const {
    return currentFPS;
}

int FPSCounter::getInstantFPS() const {
    return (deltaTime > 0.0f) ? (1.0f / deltaTime) : 0.0f;
}