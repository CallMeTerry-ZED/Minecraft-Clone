/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#pragma once

#include "World/World.h"
#include "World/TerrainGenerator.h"
#include "World/ChunkRenderer.h"
#include <glm/glm.hpp>
#include <set>
#include <unordered_set>

namespace MinecraftClone
{
    class ChunkManager
    {
    public:
        ChunkManager();
        ~ChunkManager() = default;

        void Initialize(World* world, TerrainGenerator* terrainGenerator, ChunkRenderer* chunkRenderer);
        void Update(const glm::vec3& playerPosition);
        void Shutdown();

        // Settings
        void SetRenderDistance(int distance) { m_renderDistance = distance; }
        int GetRenderDistance() const { return m_renderDistance; }
        void SetLoadDistance(int distance) { m_loadDistance = distance; }
        int GetLoadDistance() const { return m_loadDistance; }

        // Get loaded chunks info
        size_t GetLoadedChunkCount() const { return m_loadedChunks.size(); }
        std::pair<int, int> GetCurrentChunk() const { return m_currentChunk; }

    private:
        void UpdateChunks(const glm::vec3& playerPosition);
        void LoadChunk(int chunkX, int chunkZ);
        void UnloadChunk(int chunkX, int chunkZ);
        bool ShouldLoadChunk(int chunkX, int chunkZ, int centerChunkX, int centerChunkZ) const;
        bool ShouldUnloadChunk(int chunkX, int chunkZ, int centerChunkX, int centerChunkZ) const;
        int GetChunkDistance(int chunkX1, int chunkZ1, int chunkX2, int chunkZ2) const;

        World* m_world;
        TerrainGenerator* m_terrainGenerator;
        ChunkRenderer* m_chunkRenderer;

        std::set<std::pair<int, int>> m_loadedChunks;  // Chunks that are currently loaded
        std::pair<int, int> m_currentChunk;  // Current chunk player is in
        std::pair<int, int> m_lastUpdateChunk;  // Last chunk we updated for

        int m_renderDistance;  // Chunks to render (load distance)
        int m_loadDistance;    // Chunks to keep loaded (unload distance)
        bool m_initialized;
    };
}

#endif