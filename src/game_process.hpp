#pragma once
#include <string>
#include <vector>
#include <glm/glm.hpp>

using vec3 = glm::vec3;

struct Player {
    glm::vec3 position;  // x, y, z
    glm::vec3 rotation;  // yaw, pitch, roll
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