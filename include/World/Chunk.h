/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef CHUNK_H
#define CHUNK_H

#pragma once

#include "World/Block.h"
#include <glm/glm.hpp>
#include <array>
#include <cstdint>

namespace MinecraftClone
{
    // Chunk dimensions (Minecraft standard)
    constexpr int CHUNK_SIZE_X = 16;
    constexpr int CHUNK_SIZE_Y = 256;  // Height
    constexpr int CHUNK_SIZE_Z = 16;
    constexpr int CHUNK_VOLUME = CHUNK_SIZE_X * CHUNK_SIZE_Y * CHUNK_SIZE_Z;

    class Chunk
    {
    public:
        Chunk();
        Chunk(int chunkX, int chunkZ);
        ~Chunk() = default;

        // Block access
        Block& GetBlock(int x, int y, int z);
        const Block& GetBlock(int x, int y, int z) const;
        void SetBlock(int x, int y, int z, BlockType type);

        // Chunk position in world (chunk coordinates, not block coordinates)
        int GetChunkX() const { return m_chunkX; }
        int GetChunkZ() const { return m_chunkZ; }
        void SetChunkPosition(int chunkX, int chunkZ);

        // Convert world block coordinates to chunk-local coordinates
        static glm::ivec3 WorldToLocal(int worldX, int worldY, int worldZ);
        static glm::ivec3 LocalToWorld(int chunkX, int chunkZ, int localX, int localY, int localZ);

        // Check if coordinates are valid within chunk
        static bool IsValidPosition(int x, int y, int z);

        // Chunk state
        bool IsEmpty() const;
        bool NeedsMeshUpdate() const { return m_needsMeshUpdate; }
        void SetNeedsMeshUpdate(bool needsUpdate) { m_needsMeshUpdate = needsUpdate; }

    private:
        int GetIndex(int x, int y, int z) const;

        std::array<Block, CHUNK_VOLUME> m_blocks;
        int m_chunkX;
        int m_chunkZ;
        bool m_needsMeshUpdate;
    };
}

#endif