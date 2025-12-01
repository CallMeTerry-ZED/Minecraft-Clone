/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef BLOCKTYPE_H
#define BLOCKTYPE_H

#pragma once

#include <cstdint>

using std::size_t;

namespace MinecraftClone
{
    enum class BlockType : uint8_t
    {
        Air = 0,
        Grass,
        Dirt,
        Stone,
        Cobblestone,
        Sand,
        Gravel,
        Wood,
        Leaves,
        Water,
        Glass,
        Bedrock,
        Count  // Keep this last for iteration
    };

    struct BlockProperties
    {
        bool isSolid;
        bool isTransparent;
        bool isLiquid;
        bool isOpaque;
        float hardness;  // Time to break (0 = unbreakable)
        float resistance; // Blast resistance
    };

    class BlockRegistry
    {
    public:
        static void Initialize();
        static const BlockProperties& GetProperties(BlockType type);
        static bool IsSolid(BlockType type);
        static bool IsTransparent(BlockType type);
        static bool IsLiquid(BlockType type);
        static bool IsOpaque(BlockType type);

    private:
        static BlockProperties s_properties[static_cast<std::size_t>(BlockType::Count)];
        static bool s_initialized;
    };
}

#endif