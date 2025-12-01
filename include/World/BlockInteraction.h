/*
* © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef BLOCKINTERACTION_H
#define BLOCKINTERACTION_H

#pragma once

#include "World/World.h"
#include "World/Raycast.h"
#include "World/ChunkRenderer.h"
#include "World/ChunkManager.h"
#include "Core/Camera.h"
#include "World/BlockType.h"

namespace MinecraftClone
{
    class NetworkManager;  // Forward declaration
    class PhysicsManager;  // Forward declaration

    class BlockInteraction
    {
    public:
        BlockInteraction();
        ~BlockInteraction() = default;

        void Initialize(World* world, ChunkRenderer* chunkRenderer, ChunkManager* chunkManager);
        void SetNetworkManager(NetworkManager* networkManager) { m_networkManager = networkManager; }
        void SetPhysicsManager(PhysicsManager* physicsManager) { m_physicsManager = physicsManager; }
        void Update(Camera* camera, float reachDistance = 5.0f);

        // Block interaction
        void BreakBlock();
        void PlaceBlock(BlockType blockType);

        // Getters
        bool HasTarget() const { return m_lastRaycastResult.hit; }
        const RaycastResult& GetLastRaycast() const { return m_lastRaycastResult; }
        BlockType GetSelectedBlockType() const { return m_selectedBlockType; }
        void SetSelectedBlockType(BlockType type) { m_selectedBlockType = type; }

    private:
        void UpdateRaycast(Camera* camera, float reachDistance);
        void MarkChunkForUpdate(const glm::ivec3& blockPos);

        World* m_world;
        ChunkRenderer* m_chunkRenderer;
        ChunkManager* m_chunkManager;
        NetworkManager* m_networkManager;
        PhysicsManager* m_physicsManager;

        RaycastResult m_lastRaycastResult;
        BlockType m_selectedBlockType;
        bool m_initialized;
    };
}

#endif