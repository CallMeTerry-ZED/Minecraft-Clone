/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef BLOCK_H
#define BLOCK_H

#pragma once

#include "World/BlockType.h"

namespace MinecraftClone
{
    class Block
    {
    public:
        Block();
        Block(BlockType type);

        BlockType GetType() const { return m_type; }
        void SetType(BlockType type) { m_type = type; }

        bool IsAir() const { return m_type == BlockType::Air; }
        bool IsSolid() const;
        bool IsTransparent() const;
        bool IsLiquid() const;
        bool IsOpaque() const;

        // Light level (0-15)
        uint8_t GetLightLevel() const { return m_lightLevel; }
        void SetLightLevel(uint8_t level) { m_lightLevel = level; }

        // Sky light (0-15)
        uint8_t GetSkyLight() const { return m_skyLight; }
        void SetSkyLight(uint8_t level) { m_skyLight = level; }

    private:
        BlockType m_type;
        uint8_t m_lightLevel;  // Block light emission (0-15)
        uint8_t m_skyLight;     // Sky light level (0-15)
    };
}

#endif