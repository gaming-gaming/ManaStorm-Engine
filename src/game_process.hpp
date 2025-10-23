#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <btBulletDynamicsCommon.h>

using vec3 = glm::vec3;

extern float fovMultiplier; // Multiplier for FOV based on velocity, locked between 1.0 and 1.25 (90 to 112.5 degrees)
extern const float CAMERA_Y_OFFSET;

extern glm::vec3 PLAYER_SIZE;

extern bool playerSliding;

extern std::uint8_t ticksJumpHeld;
extern std::uint8_t TICKS_JUMP_HELD_MAX;

extern std::uint8_t ticksSinceLastJump;
extern const std::uint8_t TICKS_BETWEEN_JUMPS_MIN;

extern std::uint8_t coyoteTimeTicks;
extern const std::uint8_t COYOTE_TIME_TICKS_MAX;

struct Player {
    glm::vec3 size;      // width, height, depth
    glm::vec3 position;  // x, y, z
    glm::vec3 rotation;  // yaw, pitch, roll
    btRigidBody* rigidBody; // Physics body

    btCollisionShape* standingShape;
    btCollisionShape* crouchingShape;
};

class World {
public:
    // std::vector<Mesh> meshes;
    // std::vector<Enemy> enemies;
    // std::vector<Light> lights;
    // Player player;
    // Vector3 mapOffset;
};

bool isPlayerOnGround();
bool setTmap(const std::string& filePath);
void runGameProcess(float deltaTime);
void handleInput(float deltaTime);