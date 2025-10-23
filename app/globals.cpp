#include "globals.h"
ImFont* monocraft = nullptr;
const int FPS = 60;
int speed = FPS;
float gravity[2] = {0.f, 1.f};
float spawnPoint[2] = {0,0};
float spawnRadius = 10.f;
int dockedSizeX;
float simSpeed = .25;
unsigned int collissionCount = 0;
float stiffness = .25f;