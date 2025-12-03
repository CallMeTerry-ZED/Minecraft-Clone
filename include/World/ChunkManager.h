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
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <atomic>

namespace MinecraftClone
{
    class PhysicsManager;

    // Structure for chunk generation tasks
    struct ChunkGenerationTask
    {
        int chunkX;
        int chunkZ;
        bool needsTerrain;
        
        ChunkGenerationTask() : chunkX(0), chunkZ(0), needsTerrain(false) {}
        ChunkGenerationTask(int x, int z, bool terrain) : chunkX(x), chunkZ(z), needsTerrain(terrain) {}
    };

    // Structure for completed chunk meshes
    struct CompletedChunkMesh
    {
        int chunkX;
        int chunkZ;
        std::unique_ptr<ChunkMesh> mesh;
        
        CompletedChunkMesh(int x, int z, std::unique_ptr<ChunkMesh> m) 
            : chunkX(x), chunkZ(z), mesh(std::move(m)) {}
    };

    class ChunkManager
    {
    public:
        ChunkManager();
        ~ChunkManager();

        void Initialize(World* world, TerrainGenerator* terrainGenerator, ChunkRenderer* chunkRenderer);
        void SetPhysicsManager(PhysicsManager* physicsManager) { m_physicsManager = physicsManager; }
        void Update(const glm::vec3& playerPosition, float deltaTime);
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
        void ProcessChunkQueue();  // Load queued chunks gradually
        void ProcessPhysicsQueue();  // Process deferred physics collision
        void LoadChunk(int chunkX, int chunkZ, bool addPhysicsImmediately = true);
        void UnloadChunk(int chunkX, int chunkZ);
        bool ShouldLoadChunk(int chunkX, int chunkZ, int centerChunkX, int centerChunkZ) const;
        bool ShouldUnloadChunk(int chunkX, int chunkZ, int centerChunkX, int centerChunkZ) const;
        int GetChunkDistance(int chunkX1, int chunkZ1, int chunkX2, int chunkZ2) const;

        World* m_world;
        TerrainGenerator* m_terrainGenerator;
        ChunkRenderer* m_chunkRenderer;
        PhysicsManager* m_physicsManager;

        std::set<std::pair<int, int>> m_loadedChunks;  // Chunks that are currently loaded
        std::vector<std::pair<int, int>> m_chunksToLoad;  // Chunks queued for loading (ordered by priority)
        std::pair<int, int> m_currentChunk;  // Current chunk player is in
        std::pair<int, int> m_lastUpdateChunk;  // Last chunk we updated for

        int m_renderDistance;  // Chunks to render (load distance)
        int m_loadDistance;    // Chunks to keep loaded (unload distance)
        bool m_initialized;
        
        // Throttling
        float m_lastUpdateTime;
        static constexpr float UPDATE_INTERVAL = 0.1f;  // Update chunks every 100ms
        static constexpr int MAX_CHUNKS_PER_FRAME = 2;  // Load max 2 chunks per frame
        static constexpr float MAX_FRAME_TIME_MS = 8.0f;  // Max 8ms per frame for chunk loading (target 60fps = 16.67ms)
        
        // Separate queue for physics collision (deferred to reduce frame time)
        std::set<std::pair<int, int>> m_chunksPendingPhysics;
        
        // OPTIMIZATION 6: Multi-threading for chunk generation
        std::vector<std::thread> m_workerThreads;
        std::queue<ChunkGenerationTask> m_generationQueue;
        std::queue<CompletedChunkMesh> m_completedMeshes;
        std::mutex m_generationQueueMutex;
        std::mutex m_completedMeshesMutex;
        std::condition_variable m_generationCondition;
        std::atomic<bool> m_shouldStopWorkers;
        static constexpr int NUM_WORKER_THREADS = 2;  // Number of background threads
        
        void WorkerThreadFunction();  // Background thread function
        void ProcessCompletedMeshes();  // Process completed meshes on main thread
    };
}

#endif