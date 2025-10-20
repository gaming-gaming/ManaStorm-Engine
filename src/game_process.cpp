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

float fovMultiplier = 1.0f;
// const float CAMERA_Y_OFFSET = 0.72f; // Camera height offset from player position
const float CAMERA_Y_OFFSET = 0.0f;

glm::vec3 PLAYER_SIZE = glm::vec3(0.4f, 1.8f, 0.4f);

int ticksSpaceHeld = 0;
int TICKS_SPACE_HELD_MAX = 6; // For "bunny hop" movement

int ticksSinceLastJump = 0;
const int TICKS_BETWEEN_JUMPS_MIN = 6;

int coyoteTimeTicks = 0;
const int COYOTE_TIME_TICKS_MAX = 6;

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
        g_player.size = PLAYER_SIZE;
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

// Check if player is on ground
bool isPlayerOnGround() {
    if (!g_player.rigidBody) return false;
    
    int numManifolds = g_physics->getDynamicsWorld()->getDispatcher()->getNumManifolds();
    
    for (int i = 0; i < numManifolds; i++) {
        btPersistentManifold* contactManifold = 
            g_physics->getDynamicsWorld()->getDispatcher()->getManifoldByIndexInternal(i);
        
        const btCollisionObject* objA = contactManifold->getBody0();
        const btCollisionObject* objB = contactManifold->getBody1();
        
        // Check if player is involved in this contact
        if (objA != g_player.rigidBody && objB != g_player.rigidBody) {
            continue;
        }
        
        // Check contact points
        int numContacts = contactManifold->getNumContacts();
        for (int j = 0; j < numContacts; j++) {
            btManifoldPoint& pt = contactManifold->getContactPoint(j);
            
            // Check if contact normal points upward (ground contact)
            btVector3 normal = pt.m_normalWorldOnB;
            if (objB == g_player.rigidBody) {
                normal = -normal; // Flip if player is objB
            }
            
            // If normal points up (ground below player), we're on ground
            if (normal.y() > 0.7f) { // Threshold for "upward" normal
                return true;
            }
        }
    }
    
    return false;
}

void handleInput(float deltaTime) {
    if (!g_player.rigidBody) return;
    
    // Movement parameters
    float moveSpeed = 8.0f;
    float acceleration = 20.0f;
    float airControl = 0.25f;
    float maxSpeed = 16.0f;
    float jumpForce = 5.0f;
    
    // Calculate direction vectors
    glm::vec3 forward = glm::normalize(glm::vec3(
        sin(glm::radians(g_player.rotation.y)),
        0.0f,
        -cos(glm::radians(g_player.rotation.y))
    ));
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(forward, up));
    
    btVector3 velocity = g_player.rigidBody->getLinearVelocity();
    glm::vec3 horizontalVel(velocity.x(), 0, velocity.z());

    // Adjust FOV based on horizontal speed
    float horizontalSpeed = glm::length(horizontalVel);
    fovMultiplier = glm::clamp(horizontalSpeed / maxSpeed * 0.25f, 0.0f, 0.25f) * glm::clamp(horizontalSpeed / maxSpeed * 0.25f, 0.0f, 0.25f);
    fovMultiplier = 1.0f + fovMultiplier;

    bool onGround = isPlayerOnGround();
    
    glm::vec3 moveDir(0.0f);
    bool isMoving = false;
    
    // Handle movement input
    #ifdef _WIN32 // TODO: Move some of this logic outside of Windows-specific code (e.g. jumping)
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
    
    // Normalize movement direction
    if (isMoving && glm::length(moveDir) > 0.01f) {
        moveDir = glm::normalize(moveDir);
        glm::vec3 targetVel = moveDir * moveSpeed;
        glm::vec3 velDiff = targetVel - horizontalVel;
        
        float controlFactor = onGround ? 1.0f : airControl;
        glm::vec3 force = velDiff * acceleration * controlFactor * g_player.rigidBody->getMass();
        
        g_player.rigidBody->applyCentralForce(btVector3(force.x, 0, force.z));
    } else if (onGround) { // Apply friction when no input and on ground
        float currentHorizontalSpeed = glm::length(horizontalVel);
        if (currentHorizontalSpeed > 0.1f) {
            glm::vec3 friction = horizontalVel * -10.0f * g_player.rigidBody->getMass();
            g_player.rigidBody->applyCentralForce(btVector3(friction.x, 0, friction.z));
        } else {
            g_player.rigidBody->setLinearVelocity(btVector3(0, velocity.y(), 0));
        }
    }
    
    // Clamp horizontal speed
    float currentSpeed = glm::length(horizontalVel);
    if (currentSpeed > maxSpeed) {
        glm::vec3 clampedVel = glm::normalize(horizontalVel) * maxSpeed;
        g_player.rigidBody->setLinearVelocity(btVector3(clampedVel.x, velocity.y(), clampedVel.z));
    }
    
    // Handle jumping
    // static bool wasSpacePressed = false; // No longer needed; using ticksSpaceHeld instead
    bool spacePressed = (GetAsyncKeyState(VK_SPACE) & 0x8000) != 0;
    
    // Update coyote time before jump check
    if (onGround) {
        coyoteTimeTicks = 0;
    } else {
        coyoteTimeTicks++;
    }
    
    // There are a whole lot of silly rules here to make jumping feel good, here's a summary:
    // - Space must be pressed, and held for less than TICKS_SPACE_HELD_MAX (to allow "bunny hopping", but not holding space forever)
    // - Must have waited at least TICKS_BETWEEN_JUMPS_MIN since last jump (prevents double-jumping when coyote time is active)
    // - Must be on ground, or within COYOTE_TIME_TICKS_MAX since leaving ground
    // - Y velocity cannot be too high (prevents jumping while capsule is climbing a ledge, that makes an absurdly high jump)
    if (spacePressed && (ticksSpaceHeld < TICKS_SPACE_HELD_MAX) && ticksSinceLastJump > TICKS_BETWEEN_JUMPS_MIN && (onGround || coyoteTimeTicks < COYOTE_TIME_TICKS_MAX) && velocity.y() <= 0.5f) {
        g_player.rigidBody->applyCentralImpulse(btVector3(0, jumpForce * g_player.rigidBody->getMass(), 0));
        ticksSinceLastJump = 0;
        coyoteTimeTicks = COYOTE_TIME_TICKS_MAX; // Consume coyote time
    }

    ticksSinceLastJump++;
    if (spacePressed) {
        ticksSpaceHeld++;
    } else {
        ticksSpaceHeld = 0;
    }
    // wasSpacePressed = spacePressed;
    
    // Handle mouse look
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
    
    btTransform trans;
    g_player.rigidBody->getMotionState()->getWorldTransform(trans);
    g_player.position = PhysicsManager::btToGlm(trans.getOrigin());

    SetCameraPosition(g_player.position + glm::vec3(0.0f, CAMERA_Y_OFFSET, 0.0f));
    SetCameraRotation(g_player.rotation);
}

void runGameProcess(float deltaTime) {
    if (g_physics) {
        g_physics->step(deltaTime);
    }
    handleInput(deltaTime);
    // Other game logic here
}