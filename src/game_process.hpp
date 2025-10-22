#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <btBulletDynamicsCommon.h>

using vec3 = glm::vec3;

extern float fovMultiplier; // Multiplier for FOV based on velocity, locked between 1.0 and 1.25 (90 to 112.5 degrees)
extern const float CAMERA_Y_OFFSET;

extern glm::vec3 PLAYER_SIZE;

extern int ticksJumpHeld;
extern int TICKS_JUMP_HELD_MAX;

extern int ticksSinceLastJump;
extern const int TICKS_BETWEEN_JUMPS_MIN;

extern int coyoteTimeTicks;
extern const int COYOTE_TIME_TICKS_MAX;

struct Player {
    glm::vec3 size;      // width, height, depth
    glm::vec3 position;  // x, y, z
    glm::vec3 rotation;  // yaw, pitch, roll
    btRigidBody* rigidBody; // Physics body
};

class World {
public:
    // std::vector<Mesh> meshes;
    // std::vector<Enemy> enemies;
    // std::vector<Light> lights;
    // Player player;
    // Vector3 mapOffset;
};

bool setTmap(const std::string& filePath);
void runGameProcess(float deltaTime);