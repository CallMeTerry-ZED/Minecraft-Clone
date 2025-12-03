/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "Rendering/BlockTextureRegistry.h"
#include <spdlog/spdlog.h>

namespace MinecraftClone
{
    std::unordered_map<uint32_t, std::string> BlockTextureRegistry::s_textureMap;
    std::unordered_map<uint32_t, int> BlockTextureRegistry::s_atlasIndexMap;
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

    void BlockTextureRegistry::RegisterAtlasIndex(BlockType type, BlockFace face, int atlasIndex)
    {
        uint32_t key = MakeKey(type, face);
        s_atlasIndexMap[key] = atlasIndex;
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

        // ===== TEXTURE ATLAS INDICES =====
        // 4x4 atlas layout (indices 0-15)
        
        // Grass - index 0 (top), 1 (side)
        RegisterAtlasIndex(BlockType::Grass, BlockFace::Top, 0);
        RegisterAtlasIndex(BlockType::Grass, BlockFace::Front, 1);
        RegisterAtlasIndex(BlockType::Grass, BlockFace::Back, 1);
        RegisterAtlasIndex(BlockType::Grass, BlockFace::Left, 1);
        RegisterAtlasIndex(BlockType::Grass, BlockFace::Right, 1);
        RegisterAtlasIndex(BlockType::Grass, BlockFace::Bottom, 2); // Use dirt

        // Dirt - index 2
        RegisterAtlasIndex(BlockType::Dirt, BlockFace::Top, 2);
        RegisterAtlasIndex(BlockType::Dirt, BlockFace::Front, 2);
        RegisterAtlasIndex(BlockType::Dirt, BlockFace::Back, 2);
        RegisterAtlasIndex(BlockType::Dirt, BlockFace::Left, 2);
        RegisterAtlasIndex(BlockType::Dirt, BlockFace::Right, 2);
        RegisterAtlasIndex(BlockType::Dirt, BlockFace::Bottom, 2);

        // Stone - index 3
        RegisterAtlasIndex(BlockType::Stone, BlockFace::Top, 3);
        RegisterAtlasIndex(BlockType::Stone, BlockFace::Front, 3);
        RegisterAtlasIndex(BlockType::Stone, BlockFace::Back, 3);
        RegisterAtlasIndex(BlockType::Stone, BlockFace::Left, 3);
        RegisterAtlasIndex(BlockType::Stone, BlockFace::Right, 3);
        RegisterAtlasIndex(BlockType::Stone, BlockFace::Bottom, 3);

        // Cobblestone - index 4
        RegisterAtlasIndex(BlockType::Cobblestone, BlockFace::Top, 4);
        RegisterAtlasIndex(BlockType::Cobblestone, BlockFace::Front, 4);
        RegisterAtlasIndex(BlockType::Cobblestone, BlockFace::Back, 4);
        RegisterAtlasIndex(BlockType::Cobblestone, BlockFace::Left, 4);
        RegisterAtlasIndex(BlockType::Cobblestone, BlockFace::Right, 4);
        RegisterAtlasIndex(BlockType::Cobblestone, BlockFace::Bottom, 4);

        // Sand - index 5
        RegisterAtlasIndex(BlockType::Sand, BlockFace::Top, 5);
        RegisterAtlasIndex(BlockType::Sand, BlockFace::Front, 5);
        RegisterAtlasIndex(BlockType::Sand, BlockFace::Back, 5);
        RegisterAtlasIndex(BlockType::Sand, BlockFace::Left, 5);
        RegisterAtlasIndex(BlockType::Sand, BlockFace::Right, 5);
        RegisterAtlasIndex(BlockType::Sand, BlockFace::Bottom, 5);

        // Gravel - index 6
        RegisterAtlasIndex(BlockType::Gravel, BlockFace::Top, 6);
        RegisterAtlasIndex(BlockType::Gravel, BlockFace::Front, 6);
        RegisterAtlasIndex(BlockType::Gravel, BlockFace::Back, 6);
        RegisterAtlasIndex(BlockType::Gravel, BlockFace::Left, 6);
        RegisterAtlasIndex(BlockType::Gravel, BlockFace::Right, 6);
        RegisterAtlasIndex(BlockType::Gravel, BlockFace::Bottom, 6);

        // Wood - index 7 (top/bottom), 8 (sides)
        RegisterAtlasIndex(BlockType::Wood, BlockFace::Top, 7);
        RegisterAtlasIndex(BlockType::Wood, BlockFace::Front, 8);
        RegisterAtlasIndex(BlockType::Wood, BlockFace::Back, 8);
        RegisterAtlasIndex(BlockType::Wood, BlockFace::Left, 8);
        RegisterAtlasIndex(BlockType::Wood, BlockFace::Right, 8);
        RegisterAtlasIndex(BlockType::Wood, BlockFace::Bottom, 7);

        // Leaves - index 9
        RegisterAtlasIndex(BlockType::Leaves, BlockFace::Top, 9);
        RegisterAtlasIndex(BlockType::Leaves, BlockFace::Front, 9);
        RegisterAtlasIndex(BlockType::Leaves, BlockFace::Back, 9);
        RegisterAtlasIndex(BlockType::Leaves, BlockFace::Left, 9);
        RegisterAtlasIndex(BlockType::Leaves, BlockFace::Right, 9);
        RegisterAtlasIndex(BlockType::Leaves, BlockFace::Bottom, 9);

        // Water - index 10
        RegisterAtlasIndex(BlockType::Water, BlockFace::Top, 10);
        RegisterAtlasIndex(BlockType::Water, BlockFace::Front, 10);
        RegisterAtlasIndex(BlockType::Water, BlockFace::Back, 10);
        RegisterAtlasIndex(BlockType::Water, BlockFace::Left, 10);
        RegisterAtlasIndex(BlockType::Water, BlockFace::Right, 10);
        RegisterAtlasIndex(BlockType::Water, BlockFace::Bottom, 10);

        // Glass - index 11
        RegisterAtlasIndex(BlockType::Glass, BlockFace::Top, 11);
        RegisterAtlasIndex(BlockType::Glass, BlockFace::Front, 11);
        RegisterAtlasIndex(BlockType::Glass, BlockFace::Back, 11);
        RegisterAtlasIndex(BlockType::Glass, BlockFace::Left, 11);
        RegisterAtlasIndex(BlockType::Glass, BlockFace::Right, 11);
        RegisterAtlasIndex(BlockType::Glass, BlockFace::Bottom, 11);

        // Bedrock - index 12
        RegisterAtlasIndex(BlockType::Bedrock, BlockFace::Top, 12);
        RegisterAtlasIndex(BlockType::Bedrock, BlockFace::Front, 12);
        RegisterAtlasIndex(BlockType::Bedrock, BlockFace::Back, 12);
        RegisterAtlasIndex(BlockType::Bedrock, BlockFace::Left, 12);
        RegisterAtlasIndex(BlockType::Bedrock, BlockFace::Right, 12);
        RegisterAtlasIndex(BlockType::Bedrock, BlockFace::Bottom, 12);

        s_initialized = true;
        spdlog::info("BlockTextureRegistry initialized with {} texture mappings and {} atlas mappings", 
                     s_textureMap.size(), s_atlasIndexMap.size());
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

    int BlockTextureRegistry::GetAtlasIndex(BlockType type, BlockFace face)
    {
        if (!s_initialized)
        {
            Initialize();
        }

        uint32_t key = MakeKey(type, face);
        auto it = s_atlasIndexMap.find(key);
        if (it != s_atlasIndexMap.end())
        {
            return it->second;
        }

        // Fallback to dirt texture (index 2)
        spdlog::warn("Atlas index not found for block type {} face {}, using fallback", 
                     static_cast<int>(type), static_cast<int>(face));
        return 2;
    }

    AtlasUV BlockTextureRegistry::GetAtlasUV(BlockType type, BlockFace face)
    {
        int atlasIndex = GetAtlasIndex(type, face);
        
        // Calculate row and column from atlas index
        int row = atlasIndex / ATLAS_SIZE;
        int col = atlasIndex % ATLAS_SIZE;
        
        // Calculate UV coordinates (origin at bottom-left)
        float uMin = col * TILE_SIZE;
        float vMin = (ATLAS_SIZE - 1 - row) * TILE_SIZE; // Flip V coordinate
        float uMax = uMin + TILE_SIZE;
        float vMax = vMin + TILE_SIZE;
        
        return AtlasUV{ glm::vec2(uMin, vMin), glm::vec2(uMax, vMax) };
    }
}