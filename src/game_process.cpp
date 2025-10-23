#include "game_process.hpp"

#include <windows.h>
#include <iostream>
#include <GLFW/glfw3.h>
#include <cstdint>
#include <algorithm>

#include "tmap_parser.hpp"
#include "graphics/render.hpp"
#include "PhysicsManager.hpp"
#include "input/input_manager.hpp"

TMAPData g_mapData;
Player g_player;
PhysicsManager* g_physics = nullptr;

static POINT lastMousePos = { -1, -1 };
POINT mousePos;

float fovMultiplier = 1.0f;
const float CAMERA_Y_OFFSET = 0.72f;

glm::vec3 PLAYER_SIZE = glm::vec3(0.4f, 1.8f, 0.4f);

bool playerSliding = false;

std::uint8_t ticksJumpHeld = 0;
std::uint8_t TICKS_JUMP_HELD_MAX = 3; // For "bunny hop" movement

std::uint8_t ticksSinceLastJump = 0;
const std::uint8_t TICKS_BETWEEN_JUMPS_MIN = 6;

std::uint8_t coyoteTimeTicks = 0;
const std::uint8_t COYOTE_TIME_TICKS_MAX = 6;

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
        g_player.standingShape = g_physics->createCapsuleShape(
            g_player.size.x,  // radius
            g_player.size.y   // height
        );
        
        g_player.crouchingShape = g_physics->createCapsuleShape(
            g_player.size.x,  // radius (same)
            g_player.size.x   // height (becomes a sphere)
        );
        
        // Create player physics capsule with standing shape
        g_player.rigidBody = g_physics->createPlayerCapsule(
            g_player.position, 
            g_player.size.x,
            g_player.size.y
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
    
    // Movement parameters, TODO: Make these increase with progression
    float moveSpeed = 6.0f;
    float acceleration = 20.0f;
    float airControl = 0.25f;
    float maxSpeed = 16.0f;
    float jumpForce = 5.0f;
    
    // Calculate direction vectors
    glm::vec3 right = glm::normalize(glm::vec3(
        -sin(glm::radians(g_player.rotation.y)),
        0.0f,
        cos(glm::radians(g_player.rotation.y))
    ));
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 forward = glm::normalize(glm::cross(up, right));
    
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
    ControllerState inputState = ProcessInput();
    if (inputState.buttons.move_forward) {
        moveDir += forward;
        isMoving = true;
    }
    if (inputState.buttons.move_backward) {
        moveDir -= forward;
        isMoving = true;
    }
    if (inputState.buttons.move_left) {
        moveDir -= right;
        isMoving = true;
    }
    if (inputState.buttons.move_right) {
        moveDir += right;
        isMoving = true;
    }
    // If none of the WASD keys are pressed, check analog stick
    if (!isMoving) {
        float deadZone = 0.2f;
        if (std::abs(inputState.analog.left_x) > deadZone || std::abs(inputState.analog.left_y) > deadZone) {
            moveDir += forward * -inputState.analog.left_y; // Invert Y for typical stick behavior
            moveDir += right * inputState.analog.left_x;
            isMoving = true;
        }
    }
    // Normalize movement direction
    if (isMoving && glm::length(moveDir) > 0.01f) {
        moveDir = glm::normalize(moveDir);
        glm::vec3 targetVel = moveDir * moveSpeed;
        glm::vec3 velDiff = targetVel - horizontalVel;
        
        float controlFactor = onGround ? 1.0f : airControl;
        glm::vec3 force = velDiff * acceleration * controlFactor * g_player.rigidBody->getMass();
        
        g_player.rigidBody->applyCentralForce(btVector3(force.x, 0, force.z));
    } else if (onGround) { // Apply friction while no input and on ground
        float currentHorizontalSpeed = glm::length(horizontalVel);
        if (playerSliding) { // Reduce movement control while sliding
            currentHorizontalSpeed *= 0.5f;
        }
        if (currentHorizontalSpeed > 0.1f) {
            if (playerSliding) { // Less friction while sliding
                glm::vec3 friction = horizontalVel * -5.0f * g_player.rigidBody->getMass();
                g_player.rigidBody->applyCentralForce(btVector3(friction.x, 0, friction.z));
            } else {
                glm::vec3 friction = horizontalVel * -10.0f * g_player.rigidBody->getMass();
                g_player.rigidBody->applyCentralForce(btVector3(friction.x, 0, friction.z));
            }
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

    // Sliding logic
    if (inputState.buttons.slide && !playerSliding) {
        playerSliding = true;
        g_player.size.y = PLAYER_SIZE.x;
        
        // Calculate position offset
        glm::vec3 offset(0.0f);
        if (onGround) {
            float heightDiff = PLAYER_SIZE.y - PLAYER_SIZE.x;
            offset.y = -(heightDiff / 2.0f);
        }
        
        // Swap to crouching shape
        g_physics->swapPlayerShape(g_player.rigidBody, g_player.crouchingShape, offset);
        
    } else if (!inputState.buttons.slide && playerSliding) {
        float groundDistance = g_physics->getGroundDistance(g_player.rigidBody);
        float ceilingDistance = g_physics->getCeilingDistance(g_player.rigidBody);
        float heightDiff = PLAYER_SIZE.y - PLAYER_SIZE.x;
        bool enoughSpaceAbove = ceilingDistance > (heightDiff / 2.0f + 0.1f);
        
        if (enoughSpaceAbove) {
            g_player.size.y = PLAYER_SIZE.y;
            
            // Calculate position offset
            glm::vec3 offset(0.0f);
            if (onGround) {
                offset.y = heightDiff / 2.0f;
            } else {
                if (ceilingDistance < PLAYER_SIZE.y / 2.0f && groundDistance > PLAYER_SIZE.y / 2.0f) {
                    offset.y = -(PLAYER_SIZE.y / 2.0f - ceilingDistance + 0.1f);
                } else if (groundDistance > PLAYER_SIZE.y) {
                    // Displace player so they snap to the ground
                    offset.y = -groundDistance + PLAYER_SIZE.y + 0.1f;
                }
            }
            
            // Swap back to standing shape
            g_physics->swapPlayerShape(g_player.rigidBody, g_player.standingShape, offset);
            playerSliding = false;
        }
    }
    
    // Handle jumping
    // static bool wasSpacePressed = false; // No longer needed; using ticksSpaceHeld instead
    bool jumpPressed = inputState.buttons.jump;
    
    // Update coyote time before jump check
    if (onGround) {
        coyoteTimeTicks = 0;
    } else if (coyoteTimeTicks != COYOTE_TIME_TICKS_MAX) { // To prevent overflow
        coyoteTimeTicks++;
    }
    
    // There are a whole lot of silly rules here to make jumping feel good, here's a summary:
    // - Jump must be pressed, and held for less than TICKS_JUMP_HELD_MAX (to allow "bunny hopping", but not holding jump forever)
    // - Must have waited at least TICKS_BETWEEN_JUMPS_MIN since last jump, but ONLY if using coyote time
    // - Must be on ground, or within COYOTE_TIME_TICKS_MAX since leaving ground
    // - Y velocity cannot be too high (prevents jumping while capsule is climbing a ledge, that makes an absurdly high jump)
    bool canBunnyHop = ticksJumpHeld < TICKS_JUMP_HELD_MAX;
    bool canUseCoyoteTime = coyoteTimeTicks < COYOTE_TIME_TICKS_MAX && ticksSinceLastJump > TICKS_BETWEEN_JUMPS_MIN;
    bool isGroundedOrCoyoteTime = onGround || canUseCoyoteTime;
    bool isVelocitySafe = velocity.y() <= 0.5f;
    if (jumpPressed && canBunnyHop && isGroundedOrCoyoteTime && isVelocitySafe) {
        bool shouldJump = true;

        if (playerSliding) {
            float ceilingDistance = g_physics->getCeilingDistance(g_player.rigidBody);
            float heightDiff = PLAYER_SIZE.y - PLAYER_SIZE.x;
            bool enoughSpaceAbove = ceilingDistance > (heightDiff / 2.0f + 0.1f);
            
            if (enoughSpaceAbove) {
                g_player.size.y = PLAYER_SIZE.y;
                
                glm::vec3 offset(0.0f, heightDiff / 2.0f, 0.0f);
                g_physics->swapPlayerShape(g_player.rigidBody, g_player.standingShape, offset);
                playerSliding = false;
            } else {
                shouldJump = false;
            }
        }

        if (shouldJump) {
            g_player.rigidBody->applyCentralImpulse(btVector3(0, jumpForce * g_player.rigidBody->getMass(), 0));
            ticksSinceLastJump = 0;
            coyoteTimeTicks = COYOTE_TIME_TICKS_MAX; // Consume coyote time
        }
    }

    ticksSinceLastJump++;
    if (jumpPressed) {
        if (ticksJumpHeld < 15) { // Prevent overflow, only 4 bits are allocated for ticksJumpHeld
            ticksJumpHeld++;
        }
    } else {
        ticksJumpHeld = 0;
    }
    // wasJumpPressed = jumpPressed;
    
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
    // Right analog stick for camera look
    float lookDeadZone = 0.15f;
    float lookSensitivity = 3.0f; // Adjust this to taste
    if (std::abs(inputState.analog.right_x) > lookDeadZone || std::abs(inputState.analog.right_y) > lookDeadZone) {
        g_player.rotation.y += inputState.analog.right_x * lookSensitivity;
        g_player.rotation.x -= inputState.analog.right_y * lookSensitivity;
        
        // Clamp pitch
        if (g_player.rotation.x > 89.0f) g_player.rotation.x = 89.0f;
        if (g_player.rotation.x < -89.0f) g_player.rotation.x = -89.0f;
    }
    
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