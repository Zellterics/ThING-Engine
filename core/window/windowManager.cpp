#include <ThING/window/windowManager.h>

bool WindowManager::resizedFlag = true;

WindowManager::WindowManager(int width, int height, const char* title){
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

WindowManager::~WindowManager(){
    glfwDestroyWindow(window);
}