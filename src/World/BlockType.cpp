/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "World/BlockType.h"

namespace MinecraftClone
{
    BlockProperties BlockRegistry::s_properties[static_cast<size_t>(BlockType::Count)];
    bool BlockRegistry::s_initialized = false;

    void BlockRegistry::Initialize()
    {
        if (s_initialized)
        {
            return;
        }

        // Air
        s_properties[static_cast<size_t>(BlockType::Air)] = {
            false,  // isSolid
            true,   // isTransparent
            false,  // isLiquid
            false,  // isOpaque
            0.0f,   // hardness
            0.0f    // resistance
        };

        // Grass
        s_properties[static_cast<size_t>(BlockType::Grass)] = {
            true,   // isSolid
            false,  // isTransparent
            false,  // isLiquid
            true,   // isOpaque
            0.6f,   // hardness
            0.6f    // resistance
        };

        // Dirt
        s_properties[static_cast<size_t>(BlockType::Dirt)] = {
            true,   // isSolid
            false,  // isTransparent
            false,  // isLiquid
            true,   // isOpaque
            0.5f,   // hardness
            0.5f    // resistance
        };

        // Stone
        s_properties[static_cast<size_t>(BlockType::Stone)] = {
            true,   // isSolid
            false,  // isTransparent
            false,  // isLiquid
            true,   // isOpaque
            1.5f,   // hardness
            6.0f    // resistance
        };

        // Cobblestone
        s_properties[static_cast<size_t>(BlockType::Cobblestone)] = {
            true,   // isSolid
            false,  // isTransparent
            false,  // isLiquid
            true,   // isOpaque
            2.0f,   // hardness
            6.0f    // resistance
        };

        // Sand
        s_properties[static_cast<size_t>(BlockType::Sand)] = {
            true,   // isSolid
            false,  // isTransparent
            false,  // isLiquid
            true,   // isOpaque
            0.5f,   // hardness
            0.5f    // resistance
        };

        // Gravel
        s_properties[static_cast<size_t>(BlockType::Gravel)] = {
            true,   // isSolid
            false,  // isTransparent
            false,  // isLiquid
            true,   // isOpaque
            0.6f,   // hardness
            0.6f    // resistance
        };

        // Wood
        s_properties[static_cast<size_t>(BlockType::Wood)] = {
            true,   // isSolid
            false,  // isTransparent
            false,  // isLiquid
            true,   // isOpaque
            2.0f,   // hardness
            3.0f    // resistance
        };

        // Leaves
        s_properties[static_cast<size_t>(BlockType::Leaves)] = {
            true,   // isSolid
            true,   // isTransparent
            false,  // isLiquid
            false,  // isOpaque
            0.2f,   // hardness
            0.2f    // resistance
        };

        // Water
        s_properties[static_cast<size_t>(BlockType::Water)] = {
            false,  // isSolid
            true,   // isTransparent
            true,   // isLiquid
            false,  // isOpaque
            0.0f,   // hardness (can't break water)
            0.0f    // resistance
        };

        // Glass
        s_properties[static_cast<size_t>(BlockType::Glass)] = {
            true,   // isSolid
            true,   // isTransparent
            false,  // isLiquid
            false,  // isOpaque
            0.3f,   // hardness
            0.3f    // resistance
        };

        // Bedrock
        s_properties[static_cast<size_t>(BlockType::Bedrock)] = {
            true,   // isSolid
            false,  // isTransparent
            false,  // isLiquid
            true,   // isOpaque
            -1.0f,  // hardness (unbreakable)
            -1.0f   // resistance (unbreakable)
        };

        s_initialized = true;
    }

    const BlockProperties& BlockRegistry::GetProperties(BlockType type)
    {
        if (!s_initialized)
        {
            Initialize();
        }
        return s_properties[static_cast<size_t>(type)];
    }

    bool BlockRegistry::IsSolid(BlockType type)
    {
        return GetProperties(type).isSolid;
    }

    bool BlockRegistry::IsTransparent(BlockType type)
    {
        return GetProperties(type).isTransparent;
    }

    bool BlockRegistry::IsLiquid(BlockType type)
    {
        return GetProperties(type).isLiquid;
    }

    bool BlockRegistry::IsOpaque(BlockType type)
    {
        return GetProperties(type).isOpaque;
    }
}