/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include <spdlog/spdlog.h>
#include "World/BlockInteraction.h"
#include "Networking/NetworkManager.h"
#include "Physics/PhysicsManager.h"

namespace MinecraftClone
{
    BlockInteraction::BlockInteraction()
        : m_world(nullptr)
        , m_chunkRenderer(nullptr)
        , m_chunkManager(nullptr)
        , m_networkManager(nullptr)
        , m_physicsManager(nullptr)
        , m_selectedBlockType(BlockType::Stone)
        , m_initialized(false)
    {
        m_lastRaycastResult.hit = false;
    }

    void BlockInteraction::Initialize(World* world, ChunkRenderer* chunkRenderer, ChunkManager* chunkManager)
    {
        m_world = world;
        m_chunkRenderer = chunkRenderer;
        m_chunkManager = chunkManager;
        m_initialized = true;

        spdlog::info("BlockInteraction initialized");
    }

    void BlockInteraction::Update(Camera* camera, float reachDistance)
    {
        if (!m_initialized || !camera || !m_world)
        {
            return;
        }

        UpdateRaycast(camera, reachDistance);
    }

    void BlockInteraction::UpdateRaycast(Camera* camera, float reachDistance)
    {
        glm::vec3 origin = camera->GetPosition();
        glm::vec3 direction = camera->GetFront();

        m_lastRaycastResult = Raycast::Cast(origin, direction, m_world, reachDistance);
    }

    void BlockInteraction::BreakBlock()
    {
        if (!m_lastRaycastResult.hit || !m_world || !m_chunkRenderer)
        {
            return;
        }

        glm::ivec3 blockPos = m_lastRaycastResult.blockPosition;

        // Check if block is breakable (not bedrock)
        const Block& block = m_world->GetBlock(blockPos.x, blockPos.y, blockPos.z);
        if (block.GetType() == BlockType::Bedrock)
        {
            spdlog::info("Cannot break bedrock!");
            return;
        }

        // Old Block Type Not Used Rn
        //BlockType oldType = block.GetType();

        // Break the block (set to air)
        m_world->SetBlock(blockPos.x, blockPos.y, blockPos.z, BlockType::Air);

        // Mark chunk for mesh update
        MarkChunkForUpdate(blockPos);

        // Update physics collision
        if (m_physicsManager)
        {
            auto chunkCoords = World::GetChunkCoords(blockPos.x, blockPos.z);
            Chunk* chunk = m_world->GetChunk(chunkCoords.first, chunkCoords.second);
            if (chunk)
            {
                m_physicsManager->UpdateChunkCollision(chunk, chunkCoords.first, chunkCoords.second, m_world);
            }
        }

        // Send network message if connected
        if (m_networkManager && (m_networkManager->IsConnected() || m_networkManager->IsServerRunning()))
        {
            m_networkManager->SendBlockUpdate(blockPos.x, blockPos.y, blockPos.z, BlockType::Air, false);
        }

        spdlog::info("Broke block at ({}, {}, {})", blockPos.x, blockPos.y, blockPos.z);
    }

    void BlockInteraction::PlaceBlock(BlockType blockType)
    {
        if (!m_lastRaycastResult.hit || !m_world || !m_chunkRenderer)
        {
            return;
        }

        glm::ivec3 placePos = m_lastRaycastResult.adjacentPosition;

        // Check if position is valid (not inside player, not already occupied)
        const Block& existingBlock = m_world->GetBlock(placePos.x, placePos.y, placePos.z);
        if (!existingBlock.IsAir())
        {
            spdlog::info("Cannot place block - position already occupied!");
            return;
        }

        // Place the block
        m_world->SetBlock(placePos.x, placePos.y, placePos.z, blockType);

        // Mark chunk for mesh update
        MarkChunkForUpdate(placePos);

        // Update physics collision
        if (m_physicsManager)
        {
            auto chunkCoords = World::GetChunkCoords(placePos.x, placePos.z);
            Chunk* chunk = m_world->GetChunk(chunkCoords.first, chunkCoords.second);
            if (chunk)
            {
                m_physicsManager->UpdateChunkCollision(chunk, chunkCoords.first, chunkCoords.second, m_world);
            }
        }

        // Send network message if connected
        if (m_networkManager && (m_networkManager->IsConnected() || m_networkManager->IsServerRunning()))
        {
            m_networkManager->SendBlockUpdate(placePos.x, placePos.y, placePos.z, blockType, true);
        }

        spdlog::info("Placed block at ({}, {}, {})", placePos.x, placePos.y, placePos.z);
    }

    void BlockInteraction::MarkChunkForUpdate(const glm::ivec3& blockPos)
    {
        if (!m_world || !m_chunkRenderer)
        {
            return;
        }

        // Get chunk coordinates
        auto chunkCoords = World::GetChunkCoords(blockPos.x, blockPos.z);

        // Update the chunk mesh
        Chunk* chunk = m_world->GetChunk(chunkCoords.first, chunkCoords.second);
        if (chunk)
        {
            m_chunkRenderer->UpdateChunk(chunk, chunkCoords.first, chunkCoords.second, m_world);

            // Also update adjacent chunks if block is on chunk border
            // Check all 6 directions
            int offsets[6][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {0, 0}, {0, 0}};
            for (int i = 0; i < 6; i++)
            {
                int adjChunkX = chunkCoords.first + offsets[i][0];
                int adjChunkZ = chunkCoords.second + offsets[i][1];
                Chunk* adjChunk = m_world->GetChunk(adjChunkX, adjChunkZ);
                if (adjChunk)
                {
                    m_chunkRenderer->UpdateChunk(adjChunk, adjChunkX, adjChunkZ, m_world);
                }
            }
        }
    }
}