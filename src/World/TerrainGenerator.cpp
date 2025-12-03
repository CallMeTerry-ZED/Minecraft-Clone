/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "World/TerrainGenerator.h"
#include <spdlog/spdlog.h>
#include <algorithm>

namespace MinecraftClone
{
    TerrainGenerator::TerrainGenerator()
        : m_seed(12345)
        , m_seaLevel(64)
        , m_baseHeight(70)
        , m_heightVariation(30)
        , m_initialized(false)
    {
    }

    void TerrainGenerator::Initialize(int seed)
    {
        m_seed = seed;

        // Create main height noise using Perlin with fractal
        auto perlin = FastNoise::New<FastNoise::Perlin>();

        auto fractal = FastNoise::New<FastNoise::FractalFBm>();
        fractal->SetSource(perlin);
        fractal->SetOctaveCount(4);
        fractal->SetGain(0.5f);
        fractal->SetLacunarity(2.0f);
        fractal->SetWeightedStrength(0.0f);

        m_heightNoise = fractal;

        // Create detail noise for variation
        auto detailPerlin = FastNoise::New<FastNoise::Perlin>();

        auto detailFractal = FastNoise::New<FastNoise::FractalFBm>();
        detailFractal->SetSource(detailPerlin);
        detailFractal->SetOctaveCount(2);
        detailFractal->SetGain(0.5f);
        detailFractal->SetLacunarity(2.0f);

        m_detailNoise = detailFractal;

        m_initialized = true;
        spdlog::info("Terrain generator initialized with seed: {}", m_seed);
    }

    int TerrainGenerator::GetHeightAt(int worldX, int worldZ)
    {
        if (!m_initialized)
        {
            Initialize();
        }

        // Generate noise value at this position (seed is passed to GenSingle2D)
        float noiseValue = m_heightNoise->GenSingle2D(static_cast<float>(worldX) * 0.01f, static_cast<float>(worldZ) * 0.01f, m_seed);

        // Add detail noise
        float detailValue = m_detailNoise->GenSingle2D(static_cast<float>(worldX) * 0.05f, static_cast<float>(worldZ) * 0.05f, m_seed + 1000);
        detailValue *= 0.3f; // Scale down detail

        // Combine noises
        float combinedNoise = noiseValue + detailValue;

        // Convert from [-1, 1] range to height
        // Map to [baseHeight - variation, baseHeight + variation]
        int height = m_baseHeight + static_cast<int>(combinedNoise * m_heightVariation);

        // Clamp to reasonable values
        height = std::max(m_seaLevel - 10, std::min(height, 200));

        return height;
    }

    BlockType TerrainGenerator::GetBlockTypeForHeight(int height, int y)
    {
        if (y < 0 || y >= CHUNK_SIZE_Y)
        {
            return BlockType::Air;
        }

        if (y > height)
        {
            return BlockType::Air;
        }

        if (y == height)
        {
            // Surface layer
            if (height > m_seaLevel + 2)
            {
                return BlockType::Grass;
            }
            else if (height > m_seaLevel)
            {
                return BlockType::Sand;
            }
            else
            {
                return BlockType::Gravel;
            }
        }
        else if (y > height - 3)
        {
            // Dirt layer
            return BlockType::Dirt;
        }
        else if (y > height - 10)
        {
            // Stone layer
            return BlockType::Stone;
        }
        else
        {
            // Deep stone
            if (y == 0)
            {
                return BlockType::Bedrock;
            }
            return BlockType::Stone;
        }
    }

    void TerrainGenerator::GenerateChunk(Chunk* chunk, int chunkX, int chunkZ, World* /*world*/)
    {
        if (!chunk)
        {
            return;
        }

        if (!m_initialized)
        {
            Initialize();
        }

        // Generate height map for this chunk (need +1 for edges)
        constexpr int HEIGHTMAP_SIZE = CHUNK_SIZE_X + 1;
        std::vector<float> heightMap(HEIGHTMAP_SIZE * HEIGHTMAP_SIZE);

        // Calculate world coordinates for this chunk
        int worldStartX = chunkX * CHUNK_SIZE_X;
        int worldStartZ = chunkZ * CHUNK_SIZE_Z;

        // OPTIMIZATION 1: Batch noise generation using GenUniformGrid2D
        // Generate entire heightmap in 2 calls instead of 289 individual calls
        std::vector<float> heightNoiseData(HEIGHTMAP_SIZE * HEIGHTMAP_SIZE);
        std::vector<float> detailNoiseData(HEIGHTMAP_SIZE * HEIGHTMAP_SIZE);

        // Generate height noise (frequency 0.01)
        m_heightNoise->GenUniformGrid2D(
            heightNoiseData.data(),
            worldStartX, worldStartZ,
            HEIGHTMAP_SIZE, HEIGHTMAP_SIZE,
            0.01f, m_seed
        );

        // Generate detail noise (frequency 0.05)
        m_detailNoise->GenUniformGrid2D(
            detailNoiseData.data(),
            worldStartX, worldStartZ,
            HEIGHTMAP_SIZE, HEIGHTMAP_SIZE,
            0.05f, m_seed + 1000
        );

        // Combine noises to create heightmap
        for (int i = 0; i < HEIGHTMAP_SIZE * HEIGHTMAP_SIZE; i++)
        {
            float combinedNoise = heightNoiseData[i] + (detailNoiseData[i] * 0.3f);
            int height = m_baseHeight + static_cast<int>(combinedNoise * m_heightVariation);
            height = std::max(m_seaLevel - 10, std::min(height, 200));
            heightMap[i] = static_cast<float>(height);
        }

        // OPTIMIZATION 2: Optimize terrain filling
        // Fill chunk with blocks based on height map, using batch operations where possible
        for (int z = 0; z < CHUNK_SIZE_Z; z++)
        {
            for (int x = 0; x < CHUNK_SIZE_X; x++)
            {
                // Get height from height map (use bilinear interpolation for smoother terrain)
                float h1 = heightMap[z * HEIGHTMAP_SIZE + x];
                float h2 = heightMap[z * HEIGHTMAP_SIZE + (x + 1)];
                float h3 = heightMap[(z + 1) * HEIGHTMAP_SIZE + x];
                float h4 = heightMap[(z + 1) * HEIGHTMAP_SIZE + (x + 1)];

                // Average for this block position
                int height = static_cast<int>((h1 + h2 + h3 + h4) / 4.0f);

                if (height <= 0)
                {
                    continue; // Skip if no blocks to place
                }

                // Batch-fill stone layers (most common blocks)
                int stoneStartY = 0;
                int stoneEndY = std::min(height - 10, CHUNK_SIZE_Y - 1);
                if (stoneEndY >= stoneStartY)
                {
                    // Fill deep stone layer
                    for (int y = stoneStartY; y <= stoneEndY; y++)
                    {
                        BlockType blockType = (y == 0) ? BlockType::Bedrock : BlockType::Stone;
                        chunk->SetBlock(x, y, z, blockType);
                    }
                }

                // Fill middle stone layer (height - 10 to height - 3)
                int middleStartY = std::max(stoneEndY + 1, 0);
                int middleEndY = std::min(height - 3, CHUNK_SIZE_Y - 1);
                if (middleEndY >= middleStartY)
                {
                    for (int y = middleStartY; y <= middleEndY; y++)
                    {
                        chunk->SetBlock(x, y, z, BlockType::Stone);
                    }
                }

                // Fill dirt layer (height - 3 to height - 1)
                int dirtStartY = std::max(middleEndY + 1, 0);
                int dirtEndY = std::min(height - 1, CHUNK_SIZE_Y - 1);
                if (dirtEndY >= dirtStartY)
                {
                    for (int y = dirtStartY; y <= dirtEndY; y++)
                    {
                        chunk->SetBlock(x, y, z, BlockType::Dirt);
                    }
                }

                // Fill surface layer (height)
                if (height > 0 && height < CHUNK_SIZE_Y)
                {
                    BlockType surfaceType = GetBlockTypeForHeight(height, height);
                    chunk->SetBlock(x, height, z, surfaceType);
                }
            }
        }
    }
}