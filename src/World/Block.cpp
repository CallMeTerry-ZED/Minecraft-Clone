/*
* © 2025 ZED Interactive. All Rights Reserved.
 */

#include "World/Block.h"
#include "World/BlockType.h"

namespace MinecraftClone
{
    Block::Block() : m_type(BlockType::Air), m_lightLevel(0), m_skyLight(0)
    {
    }

    Block::Block(BlockType type) : m_type(type), m_lightLevel(0), m_skyLight(0)
    {
    }

    bool Block::IsSolid() const
    {
        return BlockRegistry::IsSolid(m_type);
    }

    bool Block::IsTransparent() const
    {
        return BlockRegistry::IsTransparent(m_type);
    }

    bool Block::IsLiquid() const
    {
        return BlockRegistry::IsLiquid(m_type);
    }

    bool Block::IsOpaque() const
    {
        return BlockRegistry::IsOpaque(m_type);
    }
}