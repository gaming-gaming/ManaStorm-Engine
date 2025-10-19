#include "game_process.hpp"

#include <windows.h>
#include <iostream>
#include <GLFW/glfw3.h>

#include "tmap_parser.hpp"
#include "graphics/render.hpp"

TMAPData g_mapData;
Player g_player;
static POINT lastMousePos = { -1, -1 };
POINT mousePos;

bool setTmap(const std::string& filePath) {
    std::cout << "setTmap: Attempting to load " << filePath << std::endl;
    
    if (loadTMAP(filePath, g_mapData)) {
        std::cout << "setTmap: Loaded successfully!" << std::endl;
        std::cout << "setTmap: Uploading meshes to renderer..." << std::endl;
        
        UploadTMAPMeshes(g_mapData);
        
        // Set player at spawn position
        g_player.position = glm::vec3(g_mapData.spawnPosition.x, 
                                      g_mapData.spawnPosition.y,
                                      g_mapData.spawnPosition.z);
        g_player.rotation = glm::vec3(0.0f, 0.0f, 0.0f);  // Looking forward
        
        std::cout << "setTmap: Player spawned at (" 
                  << g_player.position.x << ", " 
                  << g_player.position.y << ", " 
                  << g_player.position.z << ")" << std::endl;
        
        std::cout << "setTmap: Complete!" << std::endl;
        return true;
    } else {
        std::cerr << "setTmap: Failed to load TMAP" << std::endl;
        return false;
    }
}

void handleInput(float deltaTime) {
    float moveSpeed = 5.0f;

    // Calculate forward and right vectors based on the player's rotation
    glm::vec3 forward = glm::normalize(glm::vec3(
        sin(glm::radians(g_player.rotation.y)),
        0.0f,
        -cos(glm::radians(g_player.rotation.y))
    ));
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(forward, up));

    #ifdef _WIN32
    if (GetAsyncKeyState('W') & 0x8000) {
        g_player.position += right * moveSpeed * deltaTime;
    }
    if (GetAsyncKeyState('S') & 0x8000) {
        g_player.position -= right * moveSpeed * deltaTime;
    }
    if (GetAsyncKeyState('A') & 0x8000) {
        g_player.position += forward * moveSpeed * deltaTime;
    }
    if (GetAsyncKeyState('D') & 0x8000) {
        g_player.position -= forward * moveSpeed * deltaTime;
    }
    if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
        g_player.position.y += moveSpeed * deltaTime;
    }
    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
        g_player.position.y -= moveSpeed * deltaTime;
    }
    /*
    if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
        g_player.rotation.y -= 90.0f * deltaTime; // Yaw left
    }
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
        g_player.rotation.y += 90.0f * deltaTime; // Yaw right
    }
    if (GetAsyncKeyState(VK_UP) & 0x8000) {
        g_player.rotation.x += 90.0f * deltaTime; // Pitch up
    }
    if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
        g_player.rotation.x -= 90.0f * deltaTime; // Pitch down
    }
    */
    // Get mouse movement for looking around with raw input using GLFW since the cursor is locked
    double mousePosX, mousePosY;
    glfwGetCursorPos(glfwGetCurrentContext(), &mousePosX, &mousePosY);
    mousePos.x = static_cast<LONG>(mousePosX);
    mousePos.y = static_cast<LONG>(mousePosY);
    if (lastMousePos.x != -1 && lastMousePos.y != -1)
    {
        float mouseSensitivity = 0.1f;
        float deltaX = static_cast<float>(mousePos.x - lastMousePos.x);
        float deltaY = static_cast<float>(mousePos.y - lastMousePos.y);

        g_player.rotation.y += deltaX * mouseSensitivity; // Yaw
        g_player.rotation.x -= deltaY * mouseSensitivity; // Pitch

        // Clamp pitch to avoid flipping
        if (g_player.rotation.x > 89.0f) g_player.rotation.x = 89.0f;
        if (g_player.rotation.x < -89.0f) g_player.rotation.x = -89.0f;
    }
    lastMousePos.x = mousePos.x;
    lastMousePos.y = mousePos.y;
    #endif

    // Update the renderer's camera to match player position
    SetCameraPosition(g_player.position);
    SetCameraRotation(g_player.rotation);
}

void runGameProcess(float deltaTime) {
    handleInput(deltaTime);
    // Other game logic here
}