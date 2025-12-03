/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "World/ChunkMeshGenerator.h"
#include "Rendering/BlockTextureRegistry.h"
#include "World/World.h"
#include "World/BlockType.h"
#include <glm/glm.hpp>

namespace MinecraftClone
{
    glm::vec3 ChunkMeshGenerator::GetBlockColor(BlockType type)
    {
        switch (type)
        {
            case BlockType::Grass:
                return glm::vec3(0.2f, 0.8f, 0.2f);  // Green
            case BlockType::Dirt:
                return glm::vec3(0.6f, 0.4f, 0.2f);  // Brown
            case BlockType::Stone:
                return glm::vec3(0.5f, 0.5f, 0.5f);  // Gray
            case BlockType::Cobblestone:
                return glm::vec3(0.4f, 0.4f, 0.4f);  // Dark gray
            case BlockType::Sand:
                return glm::vec3(0.9f, 0.8f, 0.6f);  // Beige
            case BlockType::Gravel:
                return glm::vec3(0.5f, 0.5f, 0.5f);  // Gray
            case BlockType::Wood:
                return glm::vec3(0.4f, 0.25f, 0.1f); // Brown
            case BlockType::Leaves:
                return glm::vec3(0.1f, 0.6f, 0.1f);  // Dark green
            case BlockType::Water:
                return glm::vec3(0.2f, 0.4f, 0.8f);  // Blue
            case BlockType::Glass:
                return glm::vec3(0.8f, 0.9f, 1.0f);  // Light blue
            case BlockType::Bedrock:
                return glm::vec3(0.1f, 0.1f, 0.1f);  // Black
            default:
                return glm::vec3(1.0f, 1.0f, 1.0f);  // White
        }
    }

    bool ChunkMeshGenerator::ShouldRenderFace(Chunk* chunk, int x, int y, int z, int faceIndex, World* world, int chunkX, int chunkZ, Chunk* neighborChunks[4])
    {
        // Face directions: 0=front(+Z), 1=back(-Z), 2=left(-X), 3=right(+X), 4=top(+Y), 5=bottom(-Y)
        int neighborX = x;
        int neighborY = y;
        int neighborZ = z;
        int neighborChunkX = chunkX;
        int neighborChunkZ = chunkZ;

        switch (faceIndex)
        {
            case 0: // Front (+Z)
                neighborZ++;
                if (neighborZ >= CHUNK_SIZE_Z)
                {
                    neighborZ = 0;
                    neighborChunkZ++;
                }
                break;
            case 1: // Back (-Z)
                neighborZ--;
                if (neighborZ < 0)
                {
                    neighborZ = CHUNK_SIZE_Z - 1;
                    neighborChunkZ--;
                }
                break;
            case 2: // Left (-X)
                neighborX--;
                if (neighborX < 0)
                {
                    neighborX = CHUNK_SIZE_X - 1;
                    neighborChunkX--;
                }
                break;
            case 3: // Right (+X)
                neighborX++;
                if (neighborX >= CHUNK_SIZE_X)
                {
                    neighborX = 0;
                    neighborChunkX++;
                }
                break;
            case 4: // Top (+Y)
                neighborY++;
                if (neighborY >= CHUNK_SIZE_Y)
                {
                    return true; // Always render top face if at world height limit
                }
                break;
            case 5: // Bottom (-Y)
                neighborY--;
                if (neighborY < 0)
                {
                    return true; // Always render bottom face if at world bottom
                }
                break;
        }

        // Check if neighbor is in different chunk
        if (neighborChunkX != chunkX || neighborChunkZ != chunkZ)
        {
            // Determine which neighbor chunk array index to use
            int neighborIdx = -1;
            if (neighborChunkZ == chunkZ + 1) neighborIdx = 0;  // Front
            else if (neighborChunkZ == chunkZ - 1) neighborIdx = 1;  // Back
            else if (neighborChunkX == chunkX - 1) neighborIdx = 2;  // Left
            else if (neighborChunkX == chunkX + 1) neighborIdx = 3;  // Right
            
            if (neighborIdx >= 0 && neighborIdx < 4 && neighborChunks[neighborIdx])
            {
                // Use cached neighbor chunk
                const Block& neighborBlock = neighborChunks[neighborIdx]->GetBlock(neighborX, neighborY, neighborZ);
                return neighborBlock.IsAir() || neighborBlock.IsTransparent();
            }
            else
            {
                // Neighbor chunk not available, conservatively render the face
                return true;
            }
        }
        else
        {
            // Neighbor is in same chunk
            const Block& neighborBlock = chunk->GetBlock(neighborX, neighborY, neighborZ);
            return neighborBlock.IsAir() || neighborBlock.IsTransparent();
        }
    }

    void ChunkMeshGenerator::AddFace(ChunkMesh* mesh, const glm::vec3& position, BlockType blockType, int faceIndex)
    {
        // Get UV coordinates from texture atlas based on block type and face
        BlockFace face = static_cast<BlockFace>(faceIndex);
        AtlasUV atlasUV = BlockTextureRegistry::GetAtlasUV(blockType, face);
        
        // Map atlas UV to face corners
        glm::vec2 uv0(atlasUV.min.x, atlasUV.min.y);
        glm::vec2 uv1(atlasUV.max.x, atlasUV.min.y);
        glm::vec2 uv2(atlasUV.max.x, atlasUV.max.y);
        glm::vec2 uv3(atlasUV.min.x, atlasUV.max.y);

        glm::vec3 normals[] = {
            glm::vec3(0.0f, 0.0f, 1.0f),   // Front
            glm::vec3(0.0f, 0.0f, -1.0f),  // Back
            glm::vec3(-1.0f, 0.0f, 0.0f),  // Left
            glm::vec3(1.0f, 0.0f, 0.0f),   // Right
            glm::vec3(0.0f, 1.0f, 0.0f),   // Top
            glm::vec3(0.0f, -1.0f, 0.0f)   // Bottom
        };

        mesh->AddFace(position, uv0, uv1, uv2, uv3, normals[faceIndex], faceIndex);
    }

    std::unique_ptr<ChunkMesh> ChunkMeshGenerator::GenerateMesh(Chunk* chunk, int chunkX, int chunkZ, World* world)
    {
        auto mesh = std::make_unique<ChunkMesh>();

        if (!chunk || !world || chunk->IsEmpty())
        {
            return mesh;
        }

        // OPTIMIZATION 4: Cache neighbor block states to reduce redundant world lookups
        // Pre-compute neighbor chunks for boundary checks
        Chunk* neighborChunks[4] = { nullptr, nullptr, nullptr, nullptr }; // Front, Back, Left, Right
        neighborChunks[0] = world->GetChunk(chunkX, chunkZ + 1);     // Front (+Z)
        neighborChunks[1] = world->GetChunk(chunkX, chunkZ - 1);     // Back (-Z)
        neighborChunks[2] = world->GetChunk(chunkX - 1, chunkZ);     // Left (-X)
        neighborChunks[3] = world->GetChunk(chunkX + 1, chunkZ);     // Right (+X)

        for (int y = 0; y < CHUNK_SIZE_Y; y++)
        {
            for (int z = 0; z < CHUNK_SIZE_Z; z++)
            {
                for (int x = 0; x < CHUNK_SIZE_X; x++)
                {
                    const Block& block = chunk->GetBlock(x, y, z);

                    if (block.IsAir())
                    {
                        continue;
                    }

                    // OPTIMIZATION: Skip fully enclosed blocks (underground optimization)
                    // Check if all 6 neighbors are solid blocks - if so, skip this block entirely
                    bool fullyEnclosed = true;
                    for (int face = 0; face < 6; face++)
                    {
                        if (ShouldRenderFace(chunk, x, y, z, face, world, chunkX, chunkZ, neighborChunks))
                        {
                            fullyEnclosed = false;
                            break;
                        }
                    }
                    
                    if (fullyEnclosed)
                    {
                        continue; // Skip this block - it's completely surrounded
                    }

                    glm::vec3 blockPos = glm::vec3(
                        static_cast<float>(chunkX * CHUNK_SIZE_X + x),
                        static_cast<float>(y),
                        static_cast<float>(chunkZ * CHUNK_SIZE_Z + z)
                    );

                    // Generate visible faces
                    for (int face = 0; face < 6; face++)
                    {
                        bool shouldRender = false;
                        int neighborX = x;
                        int neighborY = y;
                        int neighborZ = z;

                        switch (face)
                        {
                            case 0: // Front (+Z)
                                neighborZ++;
                                if (neighborZ >= CHUNK_SIZE_Z)
                                {
                                    // Check neighbor chunk - assume air if not available (conservative)
                                    if (neighborChunks[0])
                                    {
                                        const Block& neighborBlock = neighborChunks[0]->GetBlock(x, y, 0);
                                        shouldRender = neighborBlock.IsAir() || neighborBlock.IsTransparent();
                                    }
                                    else
                                    {
                                        // No neighbor chunk loaded, assume air (conservative for rendering)
                                        shouldRender = true;
                                    }
                                }
                                else
                                {
                                    const Block& neighborBlock = chunk->GetBlock(neighborX, neighborY, neighborZ);
                                    shouldRender = neighborBlock.IsAir() || neighborBlock.IsTransparent();
                                }
                                break;
                            case 1: // Back (-Z)
                                neighborZ--;
                                if (neighborZ < 0)
                                {
                                    if (neighborChunks[1])
                                    {
                                        const Block& neighborBlock = neighborChunks[1]->GetBlock(x, y, CHUNK_SIZE_Z - 1);
                                        shouldRender = neighborBlock.IsAir() || neighborBlock.IsTransparent();
                                    }
                                    else
                                    {
                                        shouldRender = true;
                                    }
                                }
                                else
                                {
                                    const Block& neighborBlock = chunk->GetBlock(neighborX, neighborY, neighborZ);
                                    shouldRender = neighborBlock.IsAir() || neighborBlock.IsTransparent();
                                }
                                break;
                            case 2: // Left (-X)
                                neighborX--;
                                if (neighborX < 0)
                                {
                                    if (neighborChunks[2])
                                    {
                                        const Block& neighborBlock = neighborChunks[2]->GetBlock(CHUNK_SIZE_X - 1, y, z);
                                        shouldRender = neighborBlock.IsAir() || neighborBlock.IsTransparent();
                                    }
                                    else
                                    {
                                        shouldRender = true;
                                    }
                                }
                                else
                                {
                                    const Block& neighborBlock = chunk->GetBlock(neighborX, neighborY, neighborZ);
                                    shouldRender = neighborBlock.IsAir() || neighborBlock.IsTransparent();
                                }
                                break;
                            case 3: // Right (+X)
                                neighborX++;
                                if (neighborX >= CHUNK_SIZE_X)
                                {
                                    if (neighborChunks[3])
                                    {
                                        const Block& neighborBlock = neighborChunks[3]->GetBlock(0, y, z);
                                        shouldRender = neighborBlock.IsAir() || neighborBlock.IsTransparent();
                                    }
                                    else
                                    {
                                        shouldRender = true;
                                    }
                                }
                                else
                                {
                                    const Block& neighborBlock = chunk->GetBlock(neighborX, neighborY, neighborZ);
                                    shouldRender = neighborBlock.IsAir() || neighborBlock.IsTransparent();
                                }
                                break;
                            case 4: // Top (+Y)
                                neighborY++;
                                if (neighborY >= CHUNK_SIZE_Y)
                                {
                                    shouldRender = true; // Always render top face at world height limit
                                }
                                else
                                {
                                    const Block& neighborBlock = chunk->GetBlock(neighborX, neighborY, neighborZ);
                                    shouldRender = neighborBlock.IsAir() || neighborBlock.IsTransparent();
                                }
                                break;
                            case 5: // Bottom (-Y)
                                neighborY--;
                                if (neighborY < 0)
                                {
                                    shouldRender = true; // Always render bottom face at world bottom
                                }
                                else
                                {
                                    const Block& neighborBlock = chunk->GetBlock(neighborX, neighborY, neighborZ);
                                    shouldRender = neighborBlock.IsAir() || neighborBlock.IsTransparent();
                                }
                                break;
                        }

                        if (shouldRender)
                        {
                            AddFace(mesh.get(), blockPos, block.GetType(), face);
                        }
                    }
                }
            }
        }

        mesh->Build();
        return mesh;
    }
}