/*
* © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef BLOCKTEXTUREREGISTRY_H
#define BLOCKTEXTUREREGISTRY_H

#pragma once

#include "World/BlockType.h"
#include <string>
#include <unordered_map>

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

    class BlockTextureRegistry
    {
    public:
        static void Initialize();
        static std::string GetTexturePath(BlockType type, BlockFace face);
        static bool HasPerFaceTextures(BlockType type);

    private:
        static void RegisterTexture(BlockType type, BlockFace face, const std::string& path);
        static std::unordered_map<uint32_t, std::string> s_textureMap;
        static bool s_initialized;

        static uint32_t MakeKey(BlockType type, BlockFace face);
    };
}

#endif