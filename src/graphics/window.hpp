#ifndef WINDOW_H
#define WINDOW_H

// Forward declaration instead of including GLFW header
struct GLFWwindow;

class Window {
public:
    Window(const char* title, bool fullscreen, int width, int height);
    ~Window();

    void update();
    bool shouldClose() const;

private:
    GLFWwindow* m_window;
};

void create_window(const char* title, bool fullscreen, int width, int height);

#endif // WINDOW_H