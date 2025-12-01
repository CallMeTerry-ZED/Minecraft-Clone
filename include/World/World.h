/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef WORLD_H
#define WORLD_H

#pragma once

#include "World/Chunk.h"
#include <unordered_map>
#include <memory>
#include <glm/glm.hpp>

namespace MinecraftClone
{
    // Hash function for chunk coordinates
    struct ChunkCoordHash
    {
        std::size_t operator()(const std::pair<int, int>& coord) const
        {
            return std::hash<int>()(coord.first) ^ (std::hash<int>()(coord.second) << 1);
        }
    };

    class World
    {
    public:
        World();
        ~World() = default;

        // Chunk management
        Chunk* GetChunk(int chunkX, int chunkZ);
        Chunk* GetOrCreateChunk(int chunkX, int chunkZ);
        void UnloadChunk(int chunkX, int chunkZ);

        // Block access (world coordinates)
        Block& GetBlock(int worldX, int worldY, int worldZ);
        const Block& GetBlock(int worldX, int worldY, int worldZ) const;
        void SetBlock(int worldX, int worldY, int worldZ, BlockType type);

        // Get chunk coordinates from world coordinates
        static std::pair<int, int> GetChunkCoords(int worldX, int worldZ);
        static glm::ivec3 GetLocalCoords(int worldX, int worldY, int worldZ);

        // Chunk iteration
        size_t GetChunkCount() const { return m_chunks.size(); }

    private:
        std::unordered_map<std::pair<int, int>, std::unique_ptr<Chunk>, ChunkCoordHash> m_chunks;
    };
}

#endif