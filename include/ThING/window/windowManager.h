#pragma once
#include <ThING/core/detail.h>
#include "GLFW/glfw3.h"
class WindowManager{
public:
    WindowManager(int width, int height, const char* title);
    ~WindowManager();
    GLFWwindow* getWindow() const {return window;}
    void getSize(int& width, int& height)const {glfwGetWindowSize(window, &width, &height);}
private:
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<ProtoThiApp*>(glfwGetWindowUserPointer(window));
        ThING::detail::setResizedFlag(*app, true);
    }
    GLFWwindow* window = nullptr;
};