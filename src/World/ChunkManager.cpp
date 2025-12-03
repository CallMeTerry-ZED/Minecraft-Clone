/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "World/ChunkManager.h"
#include "Physics/PhysicsManager.h"
#include "World/ChunkMeshGenerator.h"
#include <spdlog/spdlog.h>
#include <chrono>
#include <algorithm>

namespace MinecraftClone
{
    ChunkManager::ChunkManager()
        : m_world(nullptr)
        , m_terrainGenerator(nullptr)
        , m_chunkRenderer(nullptr)
        , m_physicsManager(nullptr)
        , m_currentChunk(0, 0)
        , m_lastUpdateChunk(INT_MAX, INT_MAX)
        , m_renderDistance(8)  // Default render distance
        , m_loadDistance(10)    // Keep chunks loaded slightly beyond render distance
        , m_initialized(false)
        , m_lastUpdateTime(0.0f)
        , m_shouldStopWorkers(false)
    {
    }

    ChunkManager::~ChunkManager()
    {
        // Ensure proper cleanup of worker threads
        Shutdown();
    }

    void ChunkManager::Initialize(World* world, TerrainGenerator* terrainGenerator, ChunkRenderer* chunkRenderer)
    {
        m_world = world;
        m_terrainGenerator = terrainGenerator;
        m_chunkRenderer = chunkRenderer;
        m_initialized = true;

        // Start worker threads for background chunk generation
        m_shouldStopWorkers = false;
        for (int i = 0; i < NUM_WORKER_THREADS; i++)
        {
            m_workerThreads.emplace_back(&ChunkManager::WorkerThreadFunction, this);
        }

        spdlog::info("ChunkManager initialized with render distance: {}, load distance: {}, {} worker threads", 
                     m_renderDistance, m_loadDistance, NUM_WORKER_THREADS);
    }

    void ChunkManager::Update(const glm::vec3& playerPosition, float deltaTime)
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

        // Throttle chunk updates to prevent excessive loading
        m_lastUpdateTime += deltaTime;
        bool shouldUpdate = (chunkCoords != m_lastUpdateChunk) || (m_lastUpdateTime >= UPDATE_INTERVAL);
        
        if (shouldUpdate)
        {
            UpdateChunks(playerPosition);
            m_lastUpdateChunk = chunkCoords;
            m_lastUpdateTime = 0.0f;
        }

        // Process queued chunk loading (gradual loading to prevent hangs)
        ProcessChunkQueue();
        
        // Process completed meshes from background threads (must be on main thread for OpenGL)
        ProcessCompletedMeshes();
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

        // Queue new chunks for loading (prioritize chunks closer to player)
        std::vector<std::pair<int, int>> chunksToQueue;
        for (const auto& chunkCoord : chunksToLoad)
        {
            // Check if already loaded or already queued
            bool alreadyLoaded = m_loadedChunks.find(chunkCoord) != m_loadedChunks.end();
            bool alreadyQueued = std::find(m_chunksToLoad.begin(), m_chunksToLoad.end(), chunkCoord) != m_chunksToLoad.end();
            
            if (!alreadyLoaded && !alreadyQueued)
            {
                chunksToQueue.push_back(chunkCoord);
            }
        }
        
        // Sort by distance from center (closer chunks first)
        std::sort(chunksToQueue.begin(), chunksToQueue.end(), 
            [centerChunkX, centerChunkZ](const std::pair<int, int>& a, const std::pair<int, int>& b) {
                int distA = std::max(std::abs(a.first - centerChunkX), std::abs(a.second - centerChunkZ));
                int distB = std::max(std::abs(b.first - centerChunkX), std::abs(b.second - centerChunkZ));
                return distA < distB;
            });
        
        // Add to queue in priority order
        m_chunksToLoad.insert(m_chunksToLoad.end(), chunksToQueue.begin(), chunksToQueue.end());

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

        // Only log when chunk count changes significantly
        static size_t lastChunkCount = 0;
        if (std::abs(static_cast<int>(m_loadedChunks.size()) - static_cast<int>(lastChunkCount)) > 5)
        {
            spdlog::info("ChunkManager: Loaded {} chunks, current chunk: ({}, {})",
                         m_loadedChunks.size(), centerChunkX, centerChunkZ);
            lastChunkCount = m_loadedChunks.size();
        }
    }

    void ChunkManager::ProcessChunkQueue()
    {
        // Use frame time budgeting to prevent lag spikes
        auto startTime = std::chrono::high_resolution_clock::now();
        int chunksLoadedThisFrame = 0;
        
        while (!m_chunksToLoad.empty() && chunksLoadedThisFrame < MAX_CHUNKS_PER_FRAME)
        {
            // Check if we've exceeded frame time budget
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - startTime).count() / 1000.0f;
            if (elapsed > MAX_FRAME_TIME_MS)
            {
                // Stop loading chunks this frame to maintain framerate
                break;
            }
            
            // Process from front of queue (highest priority)
            auto chunkCoord = m_chunksToLoad.front();
            LoadChunk(chunkCoord.first, chunkCoord.second, false);  // Don't add physics immediately
            m_chunksToLoad.erase(m_chunksToLoad.begin());
            chunksLoadedThisFrame++;
        }
        
        // Process physics collision queue separately (slower, less frequent)
        ProcessPhysicsQueue();
    }
    
    void ChunkManager::ProcessPhysicsQueue()
    {
        // Process physics collision for only 1 chunk per frame (very expensive)
        if (!m_chunksPendingPhysics.empty() && m_physicsManager)
        {
            auto it = m_chunksPendingPhysics.begin();
            int chunkX = it->first;
            int chunkZ = it->second;
            
            Chunk* chunk = m_world->GetChunk(chunkX, chunkZ);
            if (chunk)
            {
                m_physicsManager->AddChunkCollision(chunk, chunkX, chunkZ, m_world);
            }
            
            m_chunksPendingPhysics.erase(it);
        }
    }

    void ChunkManager::LoadChunk(int chunkX, int chunkZ, bool addPhysicsImmediately)
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

        // OPTIMIZATION 6: Queue chunk for background generation instead of doing it synchronously
        bool needsTerrain = chunk->IsEmpty();
        
        {
            std::lock_guard<std::mutex> lock(m_generationQueueMutex);
            m_generationQueue.push(ChunkGenerationTask(chunkX, chunkZ, needsTerrain));
        }
        m_generationCondition.notify_one();

        // Mark as loaded (will be finalized when mesh is ready)
        m_loadedChunks.insert(std::make_pair(chunkX, chunkZ));
        
        // Add physics collision to pending queue (will be processed when mesh is ready)
        if (m_physicsManager && !addPhysicsImmediately)
        {
            m_chunksPendingPhysics.insert(std::make_pair(chunkX, chunkZ));
        }
    }

    void ChunkManager::UnloadChunk(int chunkX, int chunkZ)
    {
        if (!m_world || !m_chunkRenderer)
        {
            return;
        }

        // Remove physics collision
        if (m_physicsManager)
        {
            m_physicsManager->RemoveChunkCollision(chunkX, chunkZ);
        }

        // Remove from physics pending queue if present
        m_chunksPendingPhysics.erase(std::make_pair(chunkX, chunkZ));

        // Unload from renderer
        m_chunkRenderer->UnloadChunk(chunkX, chunkZ);

        // Unload from world
        m_world->UnloadChunk(chunkX, chunkZ);

        // Remove from loaded set
        m_loadedChunks.erase(std::make_pair(chunkX, chunkZ));
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
        // Stop worker threads
        m_shouldStopWorkers = true;
        m_generationCondition.notify_all();
        
        for (auto& thread : m_workerThreads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
        m_workerThreads.clear();

        // Unload all chunks
        std::vector<std::pair<int, int>> chunksToUnload(m_loadedChunks.begin(), m_loadedChunks.end());
        for (const auto& chunkCoord : chunksToUnload)
        {
            UnloadChunk(chunkCoord.first, chunkCoord.second);
        }

        m_loadedChunks.clear();
        m_chunksToLoad.clear();
        m_chunksPendingPhysics.clear();
        m_initialized = false;

        spdlog::info("ChunkManager shut down");
    }

    void ChunkManager::WorkerThreadFunction()
    {
        while (!m_shouldStopWorkers)
        {
            ChunkGenerationTask task;
            bool hasTask = false;

            // Get task from queue
            {
                std::unique_lock<std::mutex> lock(m_generationQueueMutex);
                m_generationCondition.wait(lock, [this] { 
                    return !m_generationQueue.empty() || m_shouldStopWorkers; 
                });

                if (!m_generationQueue.empty())
                {
                    task = m_generationQueue.front();
                    m_generationQueue.pop();
                    hasTask = true;
                }
            }

            if (!hasTask)
            {
                continue;
            }

            // Get chunk (thread-safe - World should handle this)
            Chunk* chunk = m_world->GetChunk(task.chunkX, task.chunkZ);
            if (!chunk)
            {
                continue;
            }

            // Generate terrain if needed (this is thread-safe as long as TerrainGenerator doesn't modify shared state)
            if (task.needsTerrain && m_terrainGenerator)
            {
                m_terrainGenerator->GenerateChunk(chunk, task.chunkX, task.chunkZ, m_world);
            }

            // Generate mesh (this is thread-safe - ChunkMeshGenerator doesn't modify shared state)
            // Note: GenerateMesh creates the mesh data but doesn't call Build() yet
            auto mesh = ChunkMeshGenerator::GenerateMesh(chunk, task.chunkX, task.chunkZ, m_world);
            
            // Don't call Build() here - that needs to happen on main thread for OpenGL context
            // Queue completed mesh for main thread to process
            {
                std::lock_guard<std::mutex> lock(m_completedMeshesMutex);
                m_completedMeshes.push(CompletedChunkMesh(task.chunkX, task.chunkZ, std::move(mesh)));
            }
        }
    }

    void ChunkManager::ProcessCompletedMeshes()
    {
        // Process all completed meshes from background threads
        // This must run on main thread (OpenGL context)
        std::queue<CompletedChunkMesh> meshesToProcess;
        
        {
            std::lock_guard<std::mutex> lock(m_completedMeshesMutex);
            meshesToProcess.swap(m_completedMeshes);
        }

        while (!meshesToProcess.empty())
        {
            auto& completed = meshesToProcess.front();
            
            // Build mesh on main thread (OpenGL context required)
            if (completed.mesh)
            {
                completed.mesh->Build();
                
                // Store mesh directly in renderer (bypass UpdateChunk which would regenerate)
                if (m_chunkRenderer)
                {
                    // Store the pre-built mesh in renderer (mesh is already built on main thread)
                    m_chunkRenderer->SetChunkMesh(completed.chunkX, completed.chunkZ, std::move(completed.mesh));
                }
                
                // Add physics collision if pending
                if (m_physicsManager)
                {
                    auto it = m_chunksPendingPhysics.find(std::make_pair(completed.chunkX, completed.chunkZ));
                    if (it != m_chunksPendingPhysics.end())
                    {
                        Chunk* chunk = m_world->GetChunk(completed.chunkX, completed.chunkZ);
                        if (chunk)
                        {
                            m_physicsManager->AddChunkCollision(chunk, completed.chunkX, completed.chunkZ, m_world);
                        }
                        m_chunksPendingPhysics.erase(it);
                    }
                }
            }

            meshesToProcess.pop();
        }
    }
}