#pragma once
#include "imgui.h"
extern ImFont* monocraft;
extern const int FPS;
extern int speed;
extern float gravity[2];
extern float spawnPoint[2];
extern float spawnRadius;
extern int dockedSizeX;
extern float simSpeed;
extern unsigned int collissionCount;
extern float stiffness;
extern int simWidth;
extern int simHeight;

struct WindowSize{
    int width;
    int height;
    bool operator==(const WindowSize& other) const {
        if(other.height == height && other.width == width){
            return true;
        }
        return false;
    }
    bool operator!=(const WindowSize& other) const {
        if(other.height == height && other.width == width){
            return false;
        }
        return true;
    }
};