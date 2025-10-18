#include "window.hpp"

#include <windows.h>

// #include <GL/glew.h>
#include <GLFW/glfw3.h>

Window::Window(const char* title, bool fullscreen, int width, int height) {
    // Configure GLFW for OpenGL 3.3
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_window = glfwCreateWindow(width, height, title, fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

    if (!m_window) {
        MessageBoxA(nullptr, "Window creation failed.", "Fatal Error", MB_ICONERROR);
        ExitProcess(1);
    }

    glfwMakeContextCurrent(m_window);
    
    // Enable depth testing
    // glEnable(GL_DEPTH_TEST);
}

Window::~Window() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Window::update() {
    glfwPollEvents();
    glfwSwapBuffers(m_window);
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}

void create_window(const char* title, bool fullscreen, int width, int height) {
    Window* window = new Window(title, fullscreen, width, height);
}