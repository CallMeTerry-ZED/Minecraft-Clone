/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "World/Chunk.h"
#include "World/BlockType.h"
#include <spdlog/spdlog.h>

namespace MinecraftClone
{
    Chunk::Chunk() : m_chunkX(0), m_chunkZ(0), m_needsMeshUpdate(true)
    {
        // Initialize all blocks to air
        m_blocks.fill(Block(BlockType::Air));
    }

    Chunk::Chunk(int chunkX, int chunkZ) : m_chunkX(chunkX), m_chunkZ(chunkZ), m_needsMeshUpdate(true)
    {
        // Initialize all blocks to air
        m_blocks.fill(Block(BlockType::Air));
    }

    int Chunk::GetIndex(int x, int y, int z) const
    {
        // Y is height (0-255), X and Z are horizontal (0-15)
        // Index calculation: y * (16 * 16) + z * 16 + x
        return y * (CHUNK_SIZE_X * CHUNK_SIZE_Z) + z * CHUNK_SIZE_X + x;
    }

    Block& Chunk::GetBlock(int x, int y, int z)
    {
        if (!IsValidPosition(x, y, z))
        {
            spdlog::warn("Invalid chunk position: ({}, {}, {})", x, y, z);
            static Block airBlock(BlockType::Air);
            return airBlock;
        }

        return m_blocks[GetIndex(x, y, z)];
    }

    const Block& Chunk::GetBlock(int x, int y, int z) const
    {
        if (!IsValidPosition(x, y, z))
        {
            static Block airBlock(BlockType::Air);
            return airBlock;
        }

        return m_blocks[GetIndex(x, y, z)];
    }

    void Chunk::SetBlock(int x, int y, int z, BlockType type)
    {
        if (!IsValidPosition(x, y, z))
        {
            spdlog::warn("Invalid chunk position: ({}, {}, {})", x, y, z);
            return;
        }

        Block& block = m_blocks[GetIndex(x, y, z)];
        if (block.GetType() != type)
        {
            block.SetType(type);
            m_needsMeshUpdate = true;
        }
    }

    void Chunk::SetChunkPosition(int chunkX, int chunkZ)
    {
        m_chunkX = chunkX;
        m_chunkZ = chunkZ;
        m_needsMeshUpdate = true;
    }

    glm::ivec3 Chunk::WorldToLocal(int worldX, int worldY, int worldZ)
    {
        // Convert world coordinates to chunk-local coordinates
        int localX = worldX % CHUNK_SIZE_X;
        if (localX < 0) localX += CHUNK_SIZE_X;

        int localZ = worldZ % CHUNK_SIZE_Z;
        if (localZ < 0) localZ += CHUNK_SIZE_Z;

        return glm::ivec3(localX, worldY, localZ);
    }

    glm::ivec3 Chunk::LocalToWorld(int chunkX, int chunkZ, int localX, int localY, int localZ)
    {
        int worldX = chunkX * CHUNK_SIZE_X + localX;
        int worldZ = chunkZ * CHUNK_SIZE_Z + localZ;
        return glm::ivec3(worldX, localY, worldZ);
    }

    bool Chunk::IsValidPosition(int x, int y, int z)
    {
        return x >= 0 && x < CHUNK_SIZE_X &&
               y >= 0 && y < CHUNK_SIZE_Y &&
               z >= 0 && z < CHUNK_SIZE_Z;
    }

    bool Chunk::IsEmpty() const
    {
        for (const auto& block : m_blocks)
        {
            if (!block.IsAir())
            {
                return false;
            }
        }
        return true;
    }
}