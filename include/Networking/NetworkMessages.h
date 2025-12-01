/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef NETWORKMESSAGES_H
#define NETWORKMESSAGES_H

#pragma once

// Include yojimbo.h here - message classes need full definition
// NOTE: Any .cpp file that includes this header MUST include <spdlog/spdlog.h> FIRST
#include <yojimbo.h>
#include "World/BlockType.h"
#include "World/Chunk.h"
#include <glm/glm.hpp>
#include <cstdint>
#include <cstring>

namespace MinecraftClone
{
    // Message types
    enum class GameMessageType
    {
        PLAYER_POSITION,    // Unreliable - frequent position updates
        BLOCK_UPDATE,       // Reliable - block placement/breaking
        CHUNK_DATA,         // Reliable - chunk data streaming
        PLAYER_JOINED,      // Reliable - player joined
        PLAYER_LEFT,        // Reliable - player left
        COUNT
    };

    // Network channels
    enum class GameChannel
    {
        RELIABLE,      // Reliable-ordered channel
        UNRELIABLE,    // Unreliable-unordered channel
        COUNT
    };

    // Player position message (unreliable)
    class PlayerPositionMessage : public yojimbo::Message
    {
    public:
        uint32_t playerId;
        float posX, posY, posZ;
        float yaw, pitch;

        PlayerPositionMessage()
            : playerId(0)
            , posX(0.0f), posY(0.0f), posZ(0.0f)
            , yaw(0.0f), pitch(0.0f)
        {
        }

        template <typename Stream>
        bool Serialize(Stream& stream)
        {
            serialize_bits(stream, playerId, 32);
            serialize_float(stream, posX);
            serialize_float(stream, posY);
            serialize_float(stream, posZ);
            serialize_float(stream, yaw);
            serialize_float(stream, pitch);
            return true;
        }

        YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
    };

    // Block update message (reliable)
    class BlockUpdateMessage : public yojimbo::Message
    {
    public:
        int32_t blockX, blockY, blockZ;
        uint8_t blockType;  // BlockType as uint8_t
        bool isPlacement;   // true = place, false = break

        BlockUpdateMessage()
            : blockX(0), blockY(0), blockZ(0)
            , blockType(0)
            , isPlacement(false)
        {
        }

        template <typename Stream>
        bool Serialize(Stream& stream)
        {
            serialize_int(stream, blockX, -1000000, 1000000);
            serialize_int(stream, blockY, 0, 255);
            serialize_int(stream, blockZ, -1000000, 1000000);
            serialize_bits(stream, blockType, 8);
            serialize_bool(stream, isPlacement);
            return true;
        }

        YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
    };

    // Player joined message (reliable)
    class PlayerJoinedMessage : public yojimbo::Message
    {
    public:
        uint32_t playerId;
        float posX, posY, posZ;

        PlayerJoinedMessage()
            : playerId(0)
            , posX(0.0f), posY(0.0f), posZ(0.0f)
        {
        }

        template <typename Stream>
        bool Serialize(Stream& stream)
        {
            serialize_bits(stream, playerId, 32);
            serialize_float(stream, posX);
            serialize_float(stream, posY);
            serialize_float(stream, posZ);
            return true;
        }

        YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
    };

    // Chunk slice message (reliable) - sends one Y-layer slice of a chunk
    // A full chunk is split into 16 slices (one per 16-block Y layer)
    class ChunkSliceMessage : public yojimbo::Message
    {
    public:
        int32_t chunkX, chunkZ;
        uint8_t sliceY;  // Which Y slice (0-15, each slice is 16 blocks tall)
        uint8_t blockData[CHUNK_SIZE_X * CHUNK_SIZE_Z * 16];  // 16 * 16 * 16 = 4096 bytes

        ChunkSliceMessage()
            : chunkX(0), chunkZ(0), sliceY(0)
        {
            std::memset(blockData, static_cast<int>(BlockType::Air), sizeof(blockData));
        }

        template <typename Stream>
        bool Serialize(Stream& stream)
        {
            serialize_int(stream, chunkX, -10000, 10000);
            serialize_int(stream, chunkZ, -10000, 10000);
            serialize_bits(stream, sliceY, 4);  // 0-15 fits in 4 bits
            serialize_bytes(stream, blockData, sizeof(blockData));
            return true;
        }

        YOJIMBO_VIRTUAL_SERIALIZE_FUNCTIONS();
    };

    // Message factory (must be in header for YOJIMBO macros)
    YOJIMBO_MESSAGE_FACTORY_START(GameMessageFactory, (int)GameMessageType::COUNT);
    YOJIMBO_DECLARE_MESSAGE_TYPE((int)GameMessageType::PLAYER_POSITION, PlayerPositionMessage);
    YOJIMBO_DECLARE_MESSAGE_TYPE((int)GameMessageType::BLOCK_UPDATE, BlockUpdateMessage);
    YOJIMBO_DECLARE_MESSAGE_TYPE((int)GameMessageType::PLAYER_JOINED, PlayerJoinedMessage);
    YOJIMBO_DECLARE_MESSAGE_TYPE((int)GameMessageType::CHUNK_DATA, ChunkSliceMessage);
    YOJIMBO_MESSAGE_FACTORY_FINISH();
}

#endif