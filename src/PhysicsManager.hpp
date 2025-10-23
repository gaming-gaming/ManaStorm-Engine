#ifndef PHYSICS_MANAGER_HPP
#define PHYSICS_MANAGER_HPP

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <vector>
#include "tmap_parser.hpp"

static float GRAVITY = -9.81f; // -9.81 m/s² is the gravity of Earth (May change later if the game feels better with different gravity)

class PhysicsManager {
private:
    btDiscreteDynamicsWorld* dynamicsWorld;
    btBroadphaseInterface* broadphase;
    btDefaultCollisionConfiguration* collisionConfiguration;
    btCollisionDispatcher* dispatcher;
    btSequentialImpulseConstraintSolver* solver;
    
    std::vector<btCollisionShape*> collisionShapes;
    std::vector<btRigidBody*> rigidBodies;
    
public:
    PhysicsManager();
    ~PhysicsManager();
    
    // Update physics simulation
    void step(float deltaTime);
    
    // Create static collision mesh from TMAP data
    void createStaticMeshCollision(const TMAPData& mapData);
    
    // Create player capsule
    btRigidBody* createPlayerCapsule(const glm::vec3& position, float radius, float height);

    btCollisionShape* createCapsuleShape(float radius, float height);

    // Change player capsule with new dimensions
    void swapPlayerShape(btRigidBody* body, btCollisionShape* newShape, const glm::vec3& positionOffset);

    // Get distances using raycasts for unsliding logic
    float getGroundDistance(btRigidBody* body);
    float getCeilingDistance(btRigidBody* body);
    
    // Helper to convert between GLM and Bullet vectors
    static btVector3 glmToBt(const glm::vec3& v) {
        return btVector3(v.x, v.y, v.z);
    }
    
    static glm::vec3 btToGlm(const btVector3& v) {
        return glm::vec3(v.x(), v.y(), v.z());
    }

    btDiscreteDynamicsWorld* getDynamicsWorld() const;
};

#endif // PHYSICS_MANAGER_HPP