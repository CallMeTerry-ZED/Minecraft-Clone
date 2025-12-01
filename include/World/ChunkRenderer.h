/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef CHUNKRENDERER_H
#define CHUNKRENDERER_H

#pragma once

#include "World/Chunk.h"
#include "Rendering/ChunkMesh.h"
#include "Rendering/Shader.h"
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
        void RenderChunks(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
        void UnloadChunk(int chunkX, int chunkZ);
        void Shutdown();

    private:
        std::unique_ptr<Shader> m_shader;
        std::unordered_map<std::pair<int, int>, std::unique_ptr<ChunkMesh>, ChunkCoordHash> m_chunkMeshes;
        std::unordered_map<std::string, std::unique_ptr<Texture>> m_textures;
        GLuint m_textureArrayID;
        std::unordered_map<uint32_t, GLuint> m_textureIndices; // Maps block+face to texture array layer
    };
}

#endif