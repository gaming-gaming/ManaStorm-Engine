#include "PhysicsManager.hpp"
#include <algorithm>
#include <iostream>

PhysicsManager::PhysicsManager() {
    // Set up Bullet physics world
    broadphase = new btDbvtBroadphase();
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
    solver = new btSequentialImpulseConstraintSolver();
    
    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0, GRAVITY, 0));
    
    std::cout << "Physics: Initialized Bullet physics world" << std::endl;
}

PhysicsManager::~PhysicsManager() {
    // Clean up rigid bodies
    for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--) {
        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if (body && body->getMotionState()) {
            delete body->getMotionState();
        }
        dynamicsWorld->removeCollisionObject(obj);
        delete obj;
    }
    
    // Clean up collision shapes
    for (auto shape : collisionShapes) {
        delete shape;
    }
    
    delete dynamicsWorld;
    delete solver;
    delete dispatcher;
    delete collisionConfiguration;
    delete broadphase;
}

void PhysicsManager::step(float deltaTime) {
    // Step the simulation with substeps for stability
    dynamicsWorld->stepSimulation(deltaTime, 10);
}

void PhysicsManager::createStaticMeshCollision(const TMAPData& mapData) {
    std::cout << "Physics: Creating static collision meshes..." << std::endl;
    
    for (const auto& mesh : mapData.meshes) {
        // Create a triangle mesh for this mesh
        btTriangleMesh* triangleMesh = new btTriangleMesh();
        
        // Assuming triangles (vertices come in groups of 3)
        for (size_t i = 0; i + 2 <= mesh.vertices.size(); i += 3) {
            btVector3 v0(mesh.vertices[i].x, mesh.vertices[i].y, mesh.vertices[i].z);
            btVector3 v1(mesh.vertices[i+1].x, mesh.vertices[i+1].y, mesh.vertices[i+1].z);
            btVector3 v2(mesh.vertices[i+2].x, mesh.vertices[i+2].y, mesh.vertices[i+2].z);
            
            // Apply map offset
            v0 += btVector3(mapData.mapOffset.x, mapData.mapOffset.y, mapData.mapOffset.z);
            v1 += btVector3(mapData.mapOffset.x, mapData.mapOffset.y, mapData.mapOffset.z);
            v2 += btVector3(mapData.mapOffset.x, mapData.mapOffset.y, mapData.mapOffset.z);
            
            triangleMesh->addTriangle(v0, v1, v2);
        }
        
        // Create a static collision shape from the triangle mesh
        btBvhTriangleMeshShape* meshShape = new btBvhTriangleMeshShape(triangleMesh, true);
        collisionShapes.push_back(meshShape);
        
        // Create rigid body (mass = 0 means static)
        btTransform transform;
        transform.setIdentity();
        
        btDefaultMotionState* motionState = new btDefaultMotionState(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, motionState, meshShape);
        
        // Set friction for the ground
        rbInfo.m_friction = 0.8f;
        rbInfo.m_restitution = 0.0f; // No bounciness
        
        btRigidBody* body = new btRigidBody(rbInfo);
        dynamicsWorld->addRigidBody(body);
        rigidBodies.push_back(body);
        
        std::cout << "Physics:   Added collision mesh: " << mesh.name 
                  << " (" << mesh.vertices.size() / 3 << " triangles)" << std::endl;
    }
}

btRigidBody* PhysicsManager::createPlayerCapsule(const glm::vec3& position, float radius, float height) {
    float hemispheresHeight = radius * 2.0f;
    btCapsuleShape* capsuleShape = new btCapsuleShape(radius, height - hemispheresHeight);
    collisionShapes.push_back(capsuleShape);
    
    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(glmToBt(position));
    
    float mass = 70.0f;
    btVector3 localInertia(0, 0, 0);
    capsuleShape->calculateLocalInertia(mass, localInertia);
    
    btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, capsuleShape, localInertia);
    
    rbInfo.m_friction = 0.0f; // No friction, we handle it manually in game_process.cpp (to avoid wall sticking)
    rbInfo.m_restitution = 0.0f; // No bounciness
    rbInfo.m_linearDamping = 0.1f;
    rbInfo.m_angularDamping = 0.95f;
    
    btRigidBody* body = new btRigidBody(rbInfo);
    body->setAngularFactor(btVector3(0, 1, 0));
    body->setActivationState(DISABLE_DEACTIVATION);
    
    dynamicsWorld->addRigidBody(body);
    rigidBodies.push_back(body);
    
    std::cout << "Physics: Created player capsule at (" 
              << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    
    return body;
}

btCollisionShape* PhysicsManager::createCapsuleShape(float radius, float height) {
    float hemispheresHeight = radius * 2.0f;
    btCapsuleShape* capsuleShape = new btCapsuleShape(radius, height - hemispheresHeight);
    collisionShapes.push_back(capsuleShape);
    return capsuleShape;
}

// Change player capsule with new dimensions for sliding/standing
void PhysicsManager::swapPlayerShape(btRigidBody* body, btCollisionShape* newShape, const glm::vec3& positionOffset) {
    if (!body || !newShape) return;
    
    // Store current state
    btTransform trans;
    body->getMotionState()->getWorldTransform(trans);
    btVector3 velocity = body->getLinearVelocity();
    btVector3 angularVelocity = body->getAngularVelocity();
    
    // Apply position offset
    trans.setOrigin(trans.getOrigin() + glmToBt(positionOffset));
    
    // Update the collision shape
    body->setCollisionShape(newShape);
    
    // Recalculate inertia for new shape
    float mass = 1.0f / body->getInvMass(); // Get mass from inverse
    btVector3 localInertia(0, 0, 0);
    newShape->calculateLocalInertia(mass, localInertia);
    body->setMassProps(mass, localInertia);
    
    // Restore state
    body->getMotionState()->setWorldTransform(trans);
    body->setWorldTransform(trans);
    body->setLinearVelocity(velocity);
    body->setAngularVelocity(angularVelocity);
    
    // Force Bullet to update internal structures
    body->activate(true);
    dynamicsWorld->updateSingleAabb(body);
}

float PhysicsManager::getGroundDistance(btRigidBody* body) {
    btTransform trans;
    body->getMotionState()->getWorldTransform(trans);
    btVector3 start = trans.getOrigin();
    btVector3 end = start - btVector3(0, 5.0f, 0); // Cast down 5 units for ground distance check
    btCollisionWorld::ClosestRayResultCallback rayCallback(start, end);
    dynamicsWorld->rayTest(start, end, rayCallback);
    if (rayCallback.hasHit()) {
        return (start - rayCallback.m_hitPointWorld).length();
    }
    return 5.0f; // No hit, return max distance
}

float PhysicsManager::getCeilingDistance(btRigidBody* body) {
    btTransform trans;
    body->getMotionState()->getWorldTransform(trans);
    btVector3 start = trans.getOrigin();
    btVector3 end = start + btVector3(0, 5.0f, 0); // Cast up 5 units for ceiling distance check
    btCollisionWorld::ClosestRayResultCallback rayCallback(start, end);
    dynamicsWorld->rayTest(start, end, rayCallback);
    if (rayCallback.hasHit()) {
        return (start - rayCallback.m_hitPointWorld).length();
    }
    return 5.0f; // No hit, return max distance
}

btDiscreteDynamicsWorld* PhysicsManager::getDynamicsWorld() const {
    return dynamicsWorld;
}