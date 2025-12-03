/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef CHUNKMESHGENERATOR_H
#define CHUNKMESHGENERATOR_H

#pragma once

#include "World/Chunk.h"
#include "Rendering/ChunkMesh.h"
#include "World/World.h"
#include <memory>

namespace MinecraftClone
{
    class ChunkMeshGenerator
    {
    public:
        static std::unique_ptr<ChunkMesh> GenerateMesh(Chunk* chunk, int chunkX, int chunkZ, World* world);
        static void AddFace(ChunkMesh* mesh, const glm::vec3& position, BlockType blockType, int faceIndex);

    private:
        static glm::vec3 GetBlockColor(BlockType type);
        static bool ShouldRenderFace(Chunk* chunk, int x, int y, int z, int faceIndex, World* world, int chunkX, int chunkZ, Chunk* neighborChunks[4]);
    };
}

#endif