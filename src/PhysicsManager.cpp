#include "PhysicsManager.hpp"
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
        for (size_t i = 0; i + 2 < mesh.vertices.size(); i += 3) {
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
    btCapsuleShape* capsuleShape = new btCapsuleShape(radius, height);
    collisionShapes.push_back(capsuleShape);
    
    btTransform startTransform;
    startTransform.setIdentity();
    startTransform.setOrigin(glmToBt(position));
    
    float mass = 70.0f;
    btVector3 localInertia(0, 0, 0);
    capsuleShape->calculateLocalInertia(mass, localInertia);
    
    btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, capsuleShape, localInertia);
    
    // Set friction to nearly zero to prevent wall-hugging
    rbInfo.m_friction = 0.0f; // No friction - we handle it manually
    rbInfo.m_restitution = 0.0f;
    rbInfo.m_linearDamping = 0.0f;
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

btDiscreteDynamicsWorld* PhysicsManager::getDynamicsWorld() const {
    return dynamicsWorld;
}