/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef PHYSICSMANAGER_H
#define PHYSICSMANAGER_H

#pragma once

#include <btBulletDynamicsCommon.h>
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>
#include "World/World.h"

namespace MinecraftClone
{
    class World;
    class Chunk;

    class PhysicsManager
    {
    public:
        PhysicsManager();
        ~PhysicsManager();

        bool Initialize();
        void Shutdown();

        void Update(float deltaTime);

        // World management
        void AddChunkCollision(Chunk* chunk, int chunkX, int chunkZ, World* world);
        void RemoveChunkCollision(int chunkX, int chunkZ);
        void UpdateChunkCollision(Chunk* chunk, int chunkX, int chunkZ, World* world);

        // Character controller
        class CharacterController* CreateCharacterController(const glm::vec3& position);
        void RemoveCharacterController();

        // Getters
        btDiscreteDynamicsWorld* GetWorld() const { return m_dynamicsWorld.get(); }

    private:
        std::unique_ptr<btBroadphaseInterface> m_broadphase;
        std::unique_ptr<btCollisionConfiguration> m_collisionConfiguration;
        std::unique_ptr<btCollisionDispatcher> m_dispatcher;
        std::unique_ptr<btConstraintSolver> m_solver;
        std::unique_ptr<btDiscreteDynamicsWorld> m_dynamicsWorld;

        // Chunk collision mapping
        std::unordered_map<std::pair<int, int>, btRigidBody*, ChunkCoordHash> m_chunkBodies;

        // Character controller
        class CharacterController* m_characterController;
    };
}

#endif

