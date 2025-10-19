#include "input.hpp"

#include <../game_process.hpp>

// If WASD keys are pressed, += or -= to the player's position on the respective axis
void HandleInput(float deltaTime) {
    // WASD movement
    if (GetKeyState('W') & 0x8000) {
        // Move forward
        player.position.z -= 5.0f * deltaTime;
    }
    if (GetKeyState('S') & 0x8000) {
        // Move backward
        player.position.z += 5.0f * deltaTime;
    }
    if (GetKeyState('A') & 0x8000) {
        // Move left
        player.position.x -= 5.0f * deltaTime;
    }
    if (GetKeyState('D') & 0x8000) {
        // Move right
        player.position.x += 5.0f * deltaTime;
    }
}