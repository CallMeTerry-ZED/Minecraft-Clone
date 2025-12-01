/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef TERRAINGENERATOR_H
#define TERRAINGENERATOR_H

#pragma once

#include "World/BlockType.h"
#include "World/Chunk.h"
#include "World/World.h"
#include <FastNoise/FastNoise.h>
#include <memory>

namespace MinecraftClone
{
    class TerrainGenerator
    {
    public:
        TerrainGenerator();
        ~TerrainGenerator() = default;

        void Initialize(int seed = 12345);
        void GenerateChunk(Chunk* chunk, int chunkX, int chunkZ, World* world);

        // Terrain settings
        void SetSeaLevel(int level) { m_seaLevel = level; }
        void SetBaseHeight(int height) { m_baseHeight = height; }
        void SetHeightVariation(int variation) { m_heightVariation = variation; }

    private:
        int GetHeightAt(int worldX, int worldZ);
        BlockType GetBlockTypeForHeight(int height, int y);

        FastNoise::SmartNode<> m_heightNoise;
        FastNoise::SmartNode<> m_detailNoise;

        int m_seed;
        int m_seaLevel;
        int m_baseHeight;
        int m_heightVariation;
        bool m_initialized;
    };
}

#endif