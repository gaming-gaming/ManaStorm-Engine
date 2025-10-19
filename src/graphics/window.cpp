#include "window.hpp"

#include <windows.h>
#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "render.hpp"

// Static callback function for mouse button events
static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // Lock and hide cursor when left mouse button is clicked
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

// Static callback function for keyboard events
static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        // Unlock and show cursor when ESC is pressed
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

Window::Window(const char* title, bool fullscreen, int width, int height) {
    std::cout << "Window: Creating window..." << std::endl;
    
    // Configure GLFW for OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    m_width = width;  // Initialize m_width
    m_height = height; // Initialize m_height
    m_window = glfwCreateWindow(width, height, title, fullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);

    if (!m_window) {
        MessageBoxA(nullptr, "Window creation failed.", "Fatal Error", MB_ICONERROR);
        ExitProcess(1);
    }
    
    std::cout << "Window: Window created" << std::endl;

    glfwMakeContextCurrent(m_window);
    
    // Set up input callbacks
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetKeyCallback(m_window, keyCallback);
    
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

void Window::update(int width, int height) {
    RenderFrame(width, height); // Render the 3D scene
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(m_window);
}