/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "World/ChunkManager.h"
#include <spdlog/spdlog.h>

namespace MinecraftClone
{
    ChunkManager::ChunkManager()
        : m_world(nullptr)
        , m_terrainGenerator(nullptr)
        , m_chunkRenderer(nullptr)
        , m_currentChunk(0, 0)
        , m_lastUpdateChunk(INT_MAX, INT_MAX)
        , m_renderDistance(8)  // Default render distance
        , m_loadDistance(10)    // Keep chunks loaded slightly beyond render distance
        , m_initialized(false)
    {
    }

    void ChunkManager::Initialize(World* world, TerrainGenerator* terrainGenerator, ChunkRenderer* chunkRenderer)
    {
        m_world = world;
        m_terrainGenerator = terrainGenerator;
        m_chunkRenderer = chunkRenderer;
        m_initialized = true;

        spdlog::info("ChunkManager initialized with render distance: {}, load distance: {}", m_renderDistance, m_loadDistance);
    }

    void ChunkManager::Update(const glm::vec3& playerPosition)
    {
        if (!m_initialized || !m_world || !m_terrainGenerator || !m_chunkRenderer)
        {
            return;
        }

        // Convert player position to chunk coordinates
        auto chunkCoords = World::GetChunkCoords(
            static_cast<int>(playerPosition.x),
            static_cast<int>(playerPosition.z)
        );

        m_currentChunk = chunkCoords;

        // Only update if player moved to a different chunk
        if (chunkCoords != m_lastUpdateChunk)
        {
            UpdateChunks(playerPosition);
            m_lastUpdateChunk = chunkCoords;
        }
    }

    void ChunkManager::UpdateChunks(const glm::vec3& playerPosition)
    {
        auto chunkCoords = World::GetChunkCoords(
            static_cast<int>(playerPosition.x),
            static_cast<int>(playerPosition.z)
        );

        int centerChunkX = chunkCoords.first;
        int centerChunkZ = chunkCoords.second;

        // Determine which chunks should be loaded
        std::set<std::pair<int, int>> chunksToLoad;
        for (int chunkX = centerChunkX - m_renderDistance; chunkX <= centerChunkX + m_renderDistance; chunkX++)
        {
            for (int chunkZ = centerChunkZ - m_renderDistance; chunkZ <= centerChunkZ + m_renderDistance; chunkZ++)
            {
                if (ShouldLoadChunk(chunkX, chunkZ, centerChunkX, centerChunkZ))
                {
                    chunksToLoad.insert(std::make_pair(chunkX, chunkZ));
                }
            }
        }

        // Load new chunks
        for (const auto& chunkCoord : chunksToLoad)
        {
            if (m_loadedChunks.find(chunkCoord) == m_loadedChunks.end())
            {
                LoadChunk(chunkCoord.first, chunkCoord.second);
            }
        }

        // Unload chunks that are too far away
        std::vector<std::pair<int, int>> chunksToUnload;
        for (const auto& chunkCoord : m_loadedChunks)
        {
            if (ShouldUnloadChunk(chunkCoord.first, chunkCoord.second, centerChunkX, centerChunkZ))
            {
                chunksToUnload.push_back(chunkCoord);
            }
        }

        for (const auto& chunkCoord : chunksToUnload)
        {
            UnloadChunk(chunkCoord.first, chunkCoord.second);
        }

        spdlog::debug("ChunkManager: Loaded {} chunks, current chunk: ({}, {})",
                     m_loadedChunks.size(), centerChunkX, centerChunkZ);
    }

    void ChunkManager::LoadChunk(int chunkX, int chunkZ)
    {
        if (!m_world || !m_terrainGenerator || !m_chunkRenderer)
        {
            return;
        }

        // Get or create chunk
        Chunk* chunk = m_world->GetOrCreateChunk(chunkX, chunkZ);
        if (!chunk)
        {
            spdlog::warn("Failed to create chunk at ({}, {})", chunkX, chunkZ);
            return;
        }

        // Generate terrain if chunk is empty
        if (chunk->IsEmpty())
        {
            m_terrainGenerator->GenerateChunk(chunk, chunkX, chunkZ, m_world);
        }

        // Update mesh
        m_chunkRenderer->UpdateChunk(chunk, chunkX, chunkZ, m_world);

        // Mark as loaded
        m_loadedChunks.insert(std::make_pair(chunkX, chunkZ));

        spdlog::debug("Loaded chunk ({}, {})", chunkX, chunkZ);
    }

    void ChunkManager::UnloadChunk(int chunkX, int chunkZ)
    {
        if (!m_world || !m_chunkRenderer)
        {
            return;
        }

        // Unload from renderer
        m_chunkRenderer->UnloadChunk(chunkX, chunkZ);

        // Unload from world
        m_world->UnloadChunk(chunkX, chunkZ);

        // Remove from loaded set
        m_loadedChunks.erase(std::make_pair(chunkX, chunkZ));

        spdlog::debug("Unloaded chunk ({}, {})", chunkX, chunkZ);
    }

    bool ChunkManager::ShouldLoadChunk(int chunkX, int chunkZ, int centerChunkX, int centerChunkZ) const
    {
        int distance = GetChunkDistance(chunkX, chunkZ, centerChunkX, centerChunkZ);
        return distance <= m_renderDistance;
    }

    bool ChunkManager::ShouldUnloadChunk(int chunkX, int chunkZ, int centerChunkX, int centerChunkZ) const
    {
        int distance = GetChunkDistance(chunkX, chunkZ, centerChunkX, centerChunkZ);
        return distance > m_loadDistance;
    }

    int ChunkManager::GetChunkDistance(int chunkX1, int chunkZ1, int chunkX2, int chunkZ2) const
    {
        // Use Chebyshev distance (max of X and Z differences)
        int dx = std::abs(chunkX1 - chunkX2);
        int dz = std::abs(chunkZ1 - chunkZ2);
        return std::max(dx, dz);
    }

    void ChunkManager::Shutdown()
    {
        // Unload all chunks
        std::vector<std::pair<int, int>> chunksToUnload(m_loadedChunks.begin(), m_loadedChunks.end());
        for (const auto& chunkCoord : chunksToUnload)
        {
            UnloadChunk(chunkCoord.first, chunkCoord.second);
        }

        m_loadedChunks.clear();
        m_initialized = false;

        spdlog::info("ChunkManager shut down");
    }
}