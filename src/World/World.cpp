/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "World/World.h"
#include <spdlog/spdlog.h>

namespace MinecraftClone
{
    World::World()
    {
    }

    std::pair<int, int> World::GetChunkCoords(int worldX, int worldZ)
    {
        // Calculate chunk coordinates from world coordinates
        int chunkX = worldX / CHUNK_SIZE_X;
        if (worldX < 0) chunkX -= 1;  // Handle negative coordinates

        int chunkZ = worldZ / CHUNK_SIZE_Z;
        if (worldZ < 0) chunkZ -= 1;

        return std::make_pair(chunkX, chunkZ);
    }

    glm::ivec3 World::GetLocalCoords(int worldX, int worldY, int worldZ)
    {
        auto chunkCoords = GetChunkCoords(worldX, worldZ);
        return Chunk::WorldToLocal(worldX, worldY, worldZ);
    }

    Chunk* World::GetChunk(int chunkX, int chunkZ)
    {
        auto key = std::make_pair(chunkX, chunkZ);
        auto it = m_chunks.find(key);
        if (it != m_chunks.end())
        {
            return it->second.get();
        }
        return nullptr;
    }

    Chunk* World::GetOrCreateChunk(int chunkX, int chunkZ)
    {
        auto key = std::make_pair(chunkX, chunkZ);
        auto it = m_chunks.find(key);
        if (it != m_chunks.end())
        {
            return it->second.get();
        }

        // Create new chunk
        auto chunk = std::make_unique<Chunk>(chunkX, chunkZ);
        Chunk* chunkPtr = chunk.get();
        m_chunks[key] = std::move(chunk);

        return chunkPtr;
    }

    void World::UnloadChunk(int chunkX, int chunkZ)
    {
        auto key = std::make_pair(chunkX, chunkZ);
        m_chunks.erase(key);
    }

    Block& World::GetBlock(int worldX, int worldY, int worldZ)
    {
        auto chunkCoords = GetChunkCoords(worldX, worldZ);
        Chunk* chunk = GetOrCreateChunk(chunkCoords.first, chunkCoords.second);

        glm::ivec3 localCoords = Chunk::WorldToLocal(worldX, worldY, worldZ);
        return chunk->GetBlock(localCoords.x, localCoords.y, localCoords.z);
    }

    const Block& World::GetBlock(int worldX, int worldY, int worldZ) const
    {
        auto chunkCoords = GetChunkCoords(worldX, worldZ);
        Chunk* chunk = const_cast<World*>(this)->GetChunk(chunkCoords.first, chunkCoords.second);

        if (!chunk)
        {
            static Block airBlock(BlockType::Air);
            return airBlock;
        }

        glm::ivec3 localCoords = Chunk::WorldToLocal(worldX, worldY, worldZ);
        return chunk->GetBlock(localCoords.x, localCoords.y, localCoords.z);
    }

    void World::SetBlock(int worldX, int worldY, int worldZ, BlockType type)
    {
        auto chunkCoords = GetChunkCoords(worldX, worldZ);
        Chunk* chunk = GetOrCreateChunk(chunkCoords.first, chunkCoords.second);

        glm::ivec3 localCoords = Chunk::WorldToLocal(worldX, worldY, worldZ);
        chunk->SetBlock(localCoords.x, localCoords.y, localCoords.z, type);
    }
}