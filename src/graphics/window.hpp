#ifndef WINDOW_H
#define WINDOW_H

// Forward declaration instead of including GLFW header
struct GLFWwindow;
class LowResRenderer;

class Window {
public:
    Window(const char* title, bool fullscreen, int width, int height);
    ~Window();

    void update(int width, int height);
    void enterFullscreenNative();
    void exitFullscreen(int width, int height);
    bool shouldClose() const;

private:
    GLFWwindow* m_window;
    int m_width, m_height;
};

void create_window(const char* title, bool fullscreen, int width, int height);

#endif // WINDOW_H