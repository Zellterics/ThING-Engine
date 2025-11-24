#pragma once
#include "GLFW/glfw3.h"
class WindowManager{
public:
    WindowManager(int width, int height, const char* title);
    ~WindowManager();
    GLFWwindow* getWindow() const {return window;}
    void getSize(int& width, int& height)const {glfwGetWindowSize(window, &width, &height);}
    static bool resizedFlag;
private:
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        resizedFlag = true;
    }
    GLFWwindow* window = nullptr;
};