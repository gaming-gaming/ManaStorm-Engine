#ifndef PHYSICS_MANAGER_HPP
#define PHYSICS_MANAGER_HPP

#include <btBulletDynamicsCommon.h>
#include <glm/glm.hpp>
#include <vector>
#include "tmap_parser.hpp"

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
    
    // Helper to convert between GLM and Bullet vectors
    static btVector3 glmToBt(const glm::vec3& v) {
        return btVector3(v.x, v.y, v.z);
    }
    
    static glm::vec3 btToGlm(const btVector3& v) {
        return glm::vec3(v.x(), v.y(), v.z());
    }
};

#endif // PHYSICS_MANAGER_HPP