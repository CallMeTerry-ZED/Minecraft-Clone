/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "Rendering/BlockTextureRegistry.h"
#include <spdlog/spdlog.h>

namespace MinecraftClone
{
    std::unordered_map<uint32_t, std::string> BlockTextureRegistry::s_textureMap;
    bool BlockTextureRegistry::s_initialized = false;

    uint32_t BlockTextureRegistry::MakeKey(BlockType type, BlockFace face)
    {
        return (static_cast<uint32_t>(type) << 8) | static_cast<uint32_t>(face);
    }

    void BlockTextureRegistry::RegisterTexture(BlockType type, BlockFace face, const std::string& path)
    {
        uint32_t key = MakeKey(type, face);
        s_textureMap[key] = path;
    }

    void BlockTextureRegistry::Initialize()
    {
        if (s_initialized)
        {
            return;
        }

        const std::string basePath = "assets/textures/SBS-TinyTexturePack-512x512/512x512/";

        // Grass - different textures for top, sides, and bottom
        RegisterTexture(BlockType::Grass, BlockFace::Top, basePath + "Grass/Grass_01-512x512.png");
        RegisterTexture(BlockType::Grass, BlockFace::Front, basePath + "Grass/Grass_02-512x512.png");
        RegisterTexture(BlockType::Grass, BlockFace::Back, basePath + "Grass/Grass_02-512x512.png");
        RegisterTexture(BlockType::Grass, BlockFace::Left, basePath + "Grass/Grass_02-512x512.png");
        RegisterTexture(BlockType::Grass, BlockFace::Right, basePath + "Grass/Grass_02-512x512.png");
        RegisterTexture(BlockType::Grass, BlockFace::Bottom, basePath + "Tile/Tile_01-512x512.png"); // Dirt-like

        // Dirt - same texture on all faces
        RegisterTexture(BlockType::Dirt, BlockFace::Top, basePath + "Tile/Tile_01-512x512.png");
        RegisterTexture(BlockType::Dirt, BlockFace::Front, basePath + "Tile/Tile_01-512x512.png");
        RegisterTexture(BlockType::Dirt, BlockFace::Back, basePath + "Tile/Tile_01-512x512.png");
        RegisterTexture(BlockType::Dirt, BlockFace::Left, basePath + "Tile/Tile_01-512x512.png");
        RegisterTexture(BlockType::Dirt, BlockFace::Right, basePath + "Tile/Tile_01-512x512.png");
        RegisterTexture(BlockType::Dirt, BlockFace::Bottom, basePath + "Tile/Tile_01-512x512.png");

        // Stone
        RegisterTexture(BlockType::Stone, BlockFace::Top, basePath + "Bricks/Bricks_01-512x512.png");
        RegisterTexture(BlockType::Stone, BlockFace::Front, basePath + "Bricks/Bricks_01-512x512.png");
        RegisterTexture(BlockType::Stone, BlockFace::Back, basePath + "Bricks/Bricks_01-512x512.png");
        RegisterTexture(BlockType::Stone, BlockFace::Left, basePath + "Bricks/Bricks_01-512x512.png");
        RegisterTexture(BlockType::Stone, BlockFace::Right, basePath + "Bricks/Bricks_01-512x512.png");
        RegisterTexture(BlockType::Stone, BlockFace::Bottom, basePath + "Bricks/Bricks_01-512x512.png");

        // Cobblestone
        RegisterTexture(BlockType::Cobblestone, BlockFace::Top, basePath + "Bricks/Bricks_02-512x512.png");
        RegisterTexture(BlockType::Cobblestone, BlockFace::Front, basePath + "Bricks/Bricks_02-512x512.png");
        RegisterTexture(BlockType::Cobblestone, BlockFace::Back, basePath + "Bricks/Bricks_02-512x512.png");
        RegisterTexture(BlockType::Cobblestone, BlockFace::Left, basePath + "Bricks/Bricks_02-512x512.png");
        RegisterTexture(BlockType::Cobblestone, BlockFace::Right, basePath + "Bricks/Bricks_02-512x512.png");
        RegisterTexture(BlockType::Cobblestone, BlockFace::Bottom, basePath + "Bricks/Bricks_02-512x512.png");

        // Sand
        RegisterTexture(BlockType::Sand, BlockFace::Top, basePath + "Tile/Tile_02-512x512.png");
        RegisterTexture(BlockType::Sand, BlockFace::Front, basePath + "Tile/Tile_02-512x512.png");
        RegisterTexture(BlockType::Sand, BlockFace::Back, basePath + "Tile/Tile_02-512x512.png");
        RegisterTexture(BlockType::Sand, BlockFace::Left, basePath + "Tile/Tile_02-512x512.png");
        RegisterTexture(BlockType::Sand, BlockFace::Right, basePath + "Tile/Tile_02-512x512.png");
        RegisterTexture(BlockType::Sand, BlockFace::Bottom, basePath + "Tile/Tile_02-512x512.png");

        // Gravel
        RegisterTexture(BlockType::Gravel, BlockFace::Top, basePath + "Tile/Tile_03-512x512.png");
        RegisterTexture(BlockType::Gravel, BlockFace::Front, basePath + "Tile/Tile_03-512x512.png");
        RegisterTexture(BlockType::Gravel, BlockFace::Back, basePath + "Tile/Tile_03-512x512.png");
        RegisterTexture(BlockType::Gravel, BlockFace::Left, basePath + "Tile/Tile_03-512x512.png");
        RegisterTexture(BlockType::Gravel, BlockFace::Right, basePath + "Tile/Tile_03-512x512.png");
        RegisterTexture(BlockType::Gravel, BlockFace::Bottom, basePath + "Tile/Tile_03-512x512.png");

        // Wood
        RegisterTexture(BlockType::Wood, BlockFace::Top, basePath + "Wood/Wood_01-512x512.png");
        RegisterTexture(BlockType::Wood, BlockFace::Front, basePath + "Wood/Wood_02-512x512.png");
        RegisterTexture(BlockType::Wood, BlockFace::Back, basePath + "Wood/Wood_02-512x512.png");
        RegisterTexture(BlockType::Wood, BlockFace::Left, basePath + "Wood/Wood_02-512x512.png");
        RegisterTexture(BlockType::Wood, BlockFace::Right, basePath + "Wood/Wood_02-512x512.png");
        RegisterTexture(BlockType::Wood, BlockFace::Bottom, basePath + "Wood/Wood_01-512x512.png");

        // Leaves
        RegisterTexture(BlockType::Leaves, BlockFace::Top, basePath + "Grass/Grass_03-512x512.png");
        RegisterTexture(BlockType::Leaves, BlockFace::Front, basePath + "Grass/Grass_03-512x512.png");
        RegisterTexture(BlockType::Leaves, BlockFace::Back, basePath + "Grass/Grass_03-512x512.png");
        RegisterTexture(BlockType::Leaves, BlockFace::Left, basePath + "Grass/Grass_03-512x512.png");
        RegisterTexture(BlockType::Leaves, BlockFace::Right, basePath + "Grass/Grass_03-512x512.png");
        RegisterTexture(BlockType::Leaves, BlockFace::Bottom, basePath + "Grass/Grass_03-512x512.png");

        // Water
        RegisterTexture(BlockType::Water, BlockFace::Top, basePath + "Tile/Tile_04-512x512.png");
        RegisterTexture(BlockType::Water, BlockFace::Front, basePath + "Tile/Tile_04-512x512.png");
        RegisterTexture(BlockType::Water, BlockFace::Back, basePath + "Tile/Tile_04-512x512.png");
        RegisterTexture(BlockType::Water, BlockFace::Left, basePath + "Tile/Tile_04-512x512.png");
        RegisterTexture(BlockType::Water, BlockFace::Right, basePath + "Tile/Tile_04-512x512.png");
        RegisterTexture(BlockType::Water, BlockFace::Bottom, basePath + "Tile/Tile_04-512x512.png");

        // Glass
        RegisterTexture(BlockType::Glass, BlockFace::Top, basePath + "Tile/Tile_05-512x512.png");
        RegisterTexture(BlockType::Glass, BlockFace::Front, basePath + "Tile/Tile_05-512x512.png");
        RegisterTexture(BlockType::Glass, BlockFace::Back, basePath + "Tile/Tile_05-512x512.png");
        RegisterTexture(BlockType::Glass, BlockFace::Left, basePath + "Tile/Tile_05-512x512.png");
        RegisterTexture(BlockType::Glass, BlockFace::Right, basePath + "Tile/Tile_05-512x512.png");
        RegisterTexture(BlockType::Glass, BlockFace::Bottom, basePath + "Tile/Tile_05-512x512.png");

        // Bedrock
        RegisterTexture(BlockType::Bedrock, BlockFace::Top, basePath + "Bricks/Bricks_03-512x512.png");
        RegisterTexture(BlockType::Bedrock, BlockFace::Front, basePath + "Bricks/Bricks_03-512x512.png");
        RegisterTexture(BlockType::Bedrock, BlockFace::Back, basePath + "Bricks/Bricks_03-512x512.png");
        RegisterTexture(BlockType::Bedrock, BlockFace::Left, basePath + "Bricks/Bricks_03-512x512.png");
        RegisterTexture(BlockType::Bedrock, BlockFace::Right, basePath + "Bricks/Bricks_03-512x512.png");
        RegisterTexture(BlockType::Bedrock, BlockFace::Bottom, basePath + "Bricks/Bricks_03-512x512.png");

        s_initialized = true;
        spdlog::info("BlockTextureRegistry initialized with {} texture mappings", s_textureMap.size());
    }

    std::string BlockTextureRegistry::GetTexturePath(BlockType type, BlockFace face)
    {
        if (!s_initialized)
        {
            Initialize();
        }

        uint32_t key = MakeKey(type, face);
        auto it = s_textureMap.find(key);
        if (it != s_textureMap.end())
        {
            return it->second;
        }

        // Fallback to first available texture
        spdlog::warn("Texture not found for block type {} face {}, using fallback", static_cast<int>(type), static_cast<int>(face));
        return "assets/textures/SBS-TinyTexturePack-512x512/512x512/Tile/Tile_01-512x512.png";
    }

    bool BlockTextureRegistry::HasPerFaceTextures(BlockType type)
    {
        // Grass and Wood have different textures for different faces
        return type == BlockType::Grass || type == BlockType::Wood;
    }
}