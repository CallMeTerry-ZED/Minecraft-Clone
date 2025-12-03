/*
* © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef BLOCKTEXTUREREGISTRY_H
#define BLOCKTEXTUREREGISTRY_H

#pragma once

#include "World/BlockType.h"
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

namespace MinecraftClone
{
    // Face indices: 0=front(+Z), 1=back(-Z), 2=left(-X), 3=right(+X), 4=top(+Y), 5=bottom(-Y)
    enum class BlockFace : uint8_t
    {
        Front = 0,
        Back = 1,
        Left = 2,
        Right = 3,
        Top = 4,
        Bottom = 5
    };

    // Texture atlas coordinates (UV min and max for a texture region)
    struct AtlasUV
    {
        glm::vec2 min; // Bottom-left UV
        glm::vec2 max; // Top-right UV
    };

    class BlockTextureRegistry
    {
    public:
        static void Initialize();
        static std::string GetTexturePath(BlockType type, BlockFace face);
        static bool HasPerFaceTextures(BlockType type);
        
        // Texture atlas support
        static AtlasUV GetAtlasUV(BlockType type, BlockFace face);
        static int GetAtlasIndex(BlockType type, BlockFace face);

    private:
        static void RegisterTexture(BlockType type, BlockFace face, const std::string& path);
        static void RegisterAtlasIndex(BlockType type, BlockFace face, int atlasIndex);
        
        static std::unordered_map<uint32_t, std::string> s_textureMap;
        static std::unordered_map<uint32_t, int> s_atlasIndexMap;
        static bool s_initialized;

        static uint32_t MakeKey(BlockType type, BlockFace face);
        
        // Atlas is 4x4 grid (16 texture slots)
        static constexpr int ATLAS_SIZE = 4;
        static constexpr float TILE_SIZE = 1.0f / ATLAS_SIZE;
    };
}

#endif