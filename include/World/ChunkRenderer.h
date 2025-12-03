/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef CHUNKRENDERER_H
#define CHUNKRENDERER_H

#pragma once

#include "World/Chunk.h"
#include "Rendering/ChunkMesh.h"
#include "Rendering/Shader.h"
#include "Rendering/Frustum.h"
#include "World/World.h"
#include "Rendering/Texture.h"
#include "Rendering/BlockTextureRegistry.h"
#include <unordered_map>
#include <memory>

namespace MinecraftClone
{
    class World;

    class ChunkRenderer
    {
    public:
        ChunkRenderer();
        ~ChunkRenderer();

        bool Initialize();
        void UpdateChunk(Chunk* chunk, int chunkX, int chunkZ, World* world);
        void SetChunkMesh(int chunkX, int chunkZ, std::unique_ptr<ChunkMesh> mesh);  // Set pre-built mesh (for multi-threading)
        void RenderChunks(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
        void UnloadChunk(int chunkX, int chunkZ);
        void Shutdown();

    private:
        std::unique_ptr<Shader> m_shader;
        std::unordered_map<std::pair<int, int>, std::unique_ptr<ChunkMesh>, ChunkCoordHash> m_chunkMeshes;
        std::unique_ptr<Texture> m_atlasTexture;  // Single texture atlas
        Frustum m_frustum; // For frustum culling
    };
}

#endif