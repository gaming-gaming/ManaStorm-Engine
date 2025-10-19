#include "window.hpp"
#include "render.hpp"

#include <windows.h>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

Window::Window(const char* title, bool fullscreen, int width, int height) {
    std::cout << "Window: Creating window..." << std::endl;
    
    // Configure GLFW for OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    m_window = glfwCreateWindow(width, height, title, fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

    if (!m_window) {
        MessageBoxA(nullptr, "Window creation failed.", "Fatal Error", MB_ICONERROR);
        ExitProcess(1);
    }
    
    std::cout << "Window: Window created" << std::endl;

    glfwMakeContextCurrent(m_window);
    
    // Initialize GLEW
    std::cout << "Window: Initializing GLEW..." << std::endl;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW init failed: " << glewGetErrorString(err) << std::endl;
        MessageBoxA(nullptr, "GLEW initialization failed.", "Fatal Error", MB_ICONERROR);
        ExitProcess(1);
    }
    
    std::cout << "Window: GLEW initialized" << std::endl;
    std::cout << "Window: OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Initialize renderer
    std::cout << "Window: Initializing renderer..." << std::endl;
    if (!InitRenderer()) {
        MessageBoxA(nullptr, "Renderer initialization failed.", "Fatal Error", MB_ICONERROR);
        ExitProcess(1);
    }
    
    std::cout << "Window: Everything initialized successfully!" << std::endl;
}

Window::~Window() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void Window::update() {
    RenderFrame();
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}