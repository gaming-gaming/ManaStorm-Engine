#include "game_process.hpp"

#include <windows.h>
#include <iostream>
#include <GLFW/glfw3.h>

#include "tmap_parser.hpp"
#include "graphics/render.hpp"
#include "PhysicsManager.hpp"

TMAPData g_mapData;
Player g_player;
PhysicsManager* g_physics = nullptr;

static POINT lastMousePos = { -1, -1 };
POINT mousePos;

float fov_multiplier = 1.0f; // Multiplier for FOV based on velocity, locked between 1.0 and 1.25 (90 to 112.5 degrees)

bool setTmap(const std::string& filePath) {
    std::cout << "setTmap: Attempting to load " << filePath << std::endl;
    
    if (loadTMAP(filePath, g_mapData)) {
        std::cout << "setTmap: Loaded successfully!" << std::endl;
        std::cout << "setTmap: Uploading meshes to renderer..." << std::endl;
        
        UploadTMAPMeshes(g_mapData);
        
        // Initialize physics world
        if (g_physics) {
            delete g_physics;
        }
        g_physics = new PhysicsManager();
        
        // Create collision meshes for the level
        g_physics->createStaticMeshCollision(g_mapData);
        
        // Set player at spawn position
        g_player.size = glm::vec3(0.4f, 1.8f, 0.4f); // radius, height, radius
        g_player.position = glm::vec3(g_mapData.spawnPosition.x, 
                                      g_mapData.spawnPosition.y,
                                      g_mapData.spawnPosition.z);
        g_player.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        
        // Create player physics capsule
        g_player.rigidBody = g_physics->createPlayerCapsule(
            g_player.position, 
            g_player.size.x,  // radius
            g_player.size.y   // height
        );
        
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

// Check if player is on ground using raycast
bool isPlayerOnGround() {
    if (!g_player.rigidBody) return false;
    
    btTransform trans;
    g_player.rigidBody->getMotionState()->getWorldTransform(trans);
    btVector3 from = trans.getOrigin();
    btVector3 to = from - btVector3(0, g_player.size.y / 2.0f + 0.1f, 0);
    
    btCollisionWorld::ClosestRayResultCallback rayCallback(from, to);
    g_physics->step(0); // Ensure we can access the dynamics world
    
    // This is a simplified check - you'd get the dynamics world from PhysicsManager
    // For now, we'll use velocity check
    btVector3 vel = g_player.rigidBody->getLinearVelocity();
    return fabs(vel.y()) < 0.5f;
}

void handleInput(float deltaTime) {
    if (!g_player.rigidBody) return;
    
    float moveSpeed = 5.0f; // Target speed in m/s
    float acceleration = 40.0f; // How fast we reach target speed
    float airControl = 0.3f; // Reduced control in air
    float maxSpeed = 8.0f; // Terminal velocity for horizontal movement
    float jumpForce = 5.5f;
    
    // Calculate forward and right vectors based on the player's rotation
    glm::vec3 forward = glm::normalize(glm::vec3(
        sin(glm::radians(g_player.rotation.y)),
        0.0f,
        -cos(glm::radians(g_player.rotation.y))
    ));
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(forward, up));
    
    // Get current velocity
    btVector3 velocity = g_player.rigidBody->getLinearVelocity();
    glm::vec3 horizontalVel(velocity.x(), 0, velocity.z());
    // Set FOV multiplier based on horizontal speed, TODO: Make this better and smoother, but much later and maybe if when slow-mo is a mechanic
    float horizontalSpeed = glm::length(horizontalVel);
    // fov_multiplier = 1.0f + glm::clamp(horizontalSpeed / maxSpeed * 0.25f, 0.0f, 0.25f); // KEEP COMMENTED OUT FOR NOW
    
    // Check if on ground (simple check)
    bool onGround = fabs(velocity.y()) < 1.0f;
    
    // Calculate desired movement direction
    glm::vec3 moveDir(0.0f);
    bool isMoving = false;
    
    #ifdef _WIN32
    if (GetAsyncKeyState('W') & 0x8000) {
        moveDir += right;
        isMoving = true;
    }
    if (GetAsyncKeyState('S') & 0x8000) {
        moveDir -= right;
        isMoving = true;
    }
    if (GetAsyncKeyState('A') & 0x8000) {
        moveDir += forward;
        isMoving = true;
    }
    if (GetAsyncKeyState('D') & 0x8000) {
        moveDir -= forward;
        isMoving = true;
    }
    
    // Apply movement
    if (isMoving && glm::length(moveDir) > 0.01f) {
        moveDir = glm::normalize(moveDir);
        
        // Calculate desired velocity change
        glm::vec3 targetVel = moveDir * moveSpeed;
        glm::vec3 velDiff = targetVel - horizontalVel;
        
        // Apply force (stronger on ground)
        float controlFactor = onGround ? 1.0f : airControl;
        glm::vec3 force = velDiff * acceleration * controlFactor * g_player.rigidBody->getMass();
        
        g_player.rigidBody->applyCentralForce(btVector3(force.x, 0, force.z));
    } else if (onGround) {
        // Apply friction when not moving
        float currentHorizontalSpeed = glm::length(horizontalVel);
        if (currentHorizontalSpeed > 0.1f) {
            glm::vec3 friction = horizontalVel * -10.0f * g_player.rigidBody->getMass();
            g_player.rigidBody->applyCentralForce(btVector3(friction.x, 0, friction.z));
        } else {
            // Kill small velocities to prevent sliding
            g_player.rigidBody->setLinearVelocity(btVector3(0, velocity.y(), 0));
        }
    }
    
    // Clamp horizontal velocity to max speed
    float currentSpeed = glm::length(horizontalVel);
    if (currentSpeed > maxSpeed) {
        glm::vec3 clampedVel = glm::normalize(horizontalVel) * maxSpeed;
        g_player.rigidBody->setLinearVelocity(btVector3(clampedVel.x, velocity.y(), clampedVel.z));
    }
    
    // Jumping
    static bool wasSpacePressed = false;
    bool spacePressed = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
    
    if (spacePressed && !wasSpacePressed && onGround) {
        g_player.rigidBody->applyCentralImpulse(btVector3(0, jumpForce * g_player.rigidBody->getMass(), 0));
    }
    wasSpacePressed = spacePressed;
    
    // Mouse look
    double mousePosX, mousePosY;
    glfwGetCursorPos(glfwGetCurrentContext(), &mousePosX, &mousePosY);
    mousePos.x = static_cast<LONG>(mousePosX);
    mousePos.y = static_cast<LONG>(mousePosY);
    
    if (lastMousePos.x != -1 && lastMousePos.y != -1) {
        float mouseSensitivity = 0.1f;
        float deltaX = static_cast<float>(mousePos.x - lastMousePos.x);
        float deltaY = static_cast<float>(mousePos.y - lastMousePos.y);

        g_player.rotation.y += deltaX * mouseSensitivity;
        g_player.rotation.x -= deltaY * mouseSensitivity;

        // Clamp pitch
        if (g_player.rotation.x > 89.0f) g_player.rotation.x = 89.0f;
        if (g_player.rotation.x < -89.0f) g_player.rotation.x = -89.0f;
    }
    lastMousePos.x = mousePos.x;
    lastMousePos.y = mousePos.y;
    #endif
    
    // Get player position from physics
    btTransform trans;
    g_player.rigidBody->getMotionState()->getWorldTransform(trans);
    g_player.position = PhysicsManager::btToGlm(trans.getOrigin());
    
    // Update renderer camera
    SetCameraPosition(g_player.position);
    SetCameraRotation(g_player.rotation);
}

void runGameProcess(float deltaTime) {
    if (g_physics) {
        g_physics->step(deltaTime);
    }
    handleInput(deltaTime);
    // Other game logic here
}