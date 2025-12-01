/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

// CRITICAL: Include spdlog FIRST before any yojimbo headers to avoid macro conflicts
#include <spdlog/spdlog.h>

// Now include NetworkManager.h (which includes NetworkMessages.h which includes yojimbo.h)
#include "Networking/NetworkManager.h"
#include "World/World.h"
#include "World/Chunk.h"
#include "World/ChunkRenderer.h"

#include <glm/gtc/type_ptr.hpp>
#include <cstring>
#include <random>

namespace MinecraftClone
{
    const uint8_t NetworkManager::DEFAULT_PRIVATE_KEY[32] = { 0 };

    NetworkManager::NetworkManager()
        : m_isServer(false)
        , m_localPlayerId(0)
        , m_time(0.0)
    {
        // Generate random client ID - use global function
        yojimbo_random_bytes((uint8_t*)&m_localPlayerId, sizeof(uint32_t));
    }

    NetworkManager::~NetworkManager()
    {
        StopServer();
        Disconnect();
    }

    bool NetworkManager::StartServer(const char* address, int port)
    {
        if (m_server && m_server->IsRunning())
        {
            spdlog::warn("Server is already running!");
            return false;
        }

        yojimbo::Address serverAddress(address, port);
        if (!serverAddress.IsValid())
        {
            spdlog::error("Invalid server address: {}:{}", address, port);
            return false;
        }

        m_server = std::make_unique<yojimbo::Server>(
            yojimbo::GetDefaultAllocator(),
            DEFAULT_PRIVATE_KEY,
            serverAddress,
            m_config,
            m_adapter,
            m_time
        );

        const int MAX_PLAYERS = 64;
        m_server->Start(MAX_PLAYERS);

        if (!m_server->IsRunning())
        {
            spdlog::error("Failed to start server on {}:{}", address, port);
            m_server.reset();
            return false;
        }

        m_isServer = true;

        char buffer[256];
        m_server->GetAddress().ToString(buffer, sizeof(buffer));
        spdlog::info("Server started on {}", buffer);

        return true;
    }

    void NetworkManager::StopServer()
    {
        if (m_server)
        {
            m_server->Stop();
            m_server.reset();
            m_isServer = false;
            m_playerPositions.clear();
            spdlog::info("Server stopped");
        }
    }

    bool NetworkManager::ConnectToServer(const char* address, int port)
    {
        if (m_client && m_client->IsConnected())
        {
            spdlog::warn("Already connected to server!");
            return false;
        }

        yojimbo::Address serverAddress(address, port);
        if (!serverAddress.IsValid())
        {
            spdlog::error("Invalid server address: {}:{}", address, port);
            return false;
        }

        m_client = std::make_unique<yojimbo::Client>(
            yojimbo::GetDefaultAllocator(),
            yojimbo::Address("0.0.0.0"),
            m_config,
            m_adapter,
            m_time
        );

        m_client->InsecureConnect(DEFAULT_PRIVATE_KEY, m_localPlayerId, serverAddress);

        if (m_client->ConnectionFailed())
        {
            spdlog::error("Failed to connect to server");
            m_client.reset();
            return false;
        }

        m_isServer = false;
        spdlog::info("Connecting to server {}:{}...", address, port);

        return true;
    }

    void NetworkManager::Disconnect()
    {
        if (m_client)
        {
            m_client->Disconnect();
            m_client.reset();
            spdlog::info("Disconnected from server");
        }
    }

    void NetworkManager::Update(double time, float deltaTime)
    {
        m_time = time;

        if (m_isServer && m_server)
        {
            UpdateServer(time, deltaTime);
        }
        else if (m_client)
        {
            UpdateClient(time, deltaTime);
        }
    }

    void NetworkManager::UpdateServer(double time, float deltaTime)
    {
        (void)deltaTime; // Suppress unused parameter warning

        if (!m_server->IsRunning())
        {
            return;
        }

        m_server->AdvanceTime(time);
        m_server->ReceivePackets();
        ProcessServerMessages();
        m_server->SendPackets();
    }

    void NetworkManager::UpdateClient(double time, float deltaTime)
    {
        (void)deltaTime; // Suppress unused parameter warning

        if (!m_client)
        {
            return;
        }

        m_client->AdvanceTime(time);
        m_client->ReceivePackets();
        ProcessClientMessages();
        m_client->SendPackets();

        if (m_client->ConnectionFailed())
        {
            spdlog::error("Connection to server failed!");
            Disconnect();
        }
    }

    void NetworkManager::ProcessServerMessages()
    {
        const int MAX_PLAYERS = 64;
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (!m_server->IsClientConnected(i))
            {
                continue;
            }

            for (int j = 0; j < m_config.numChannels; j++)
            {
                yojimbo::Message* message = m_server->ReceiveMessage(i, j);
                while (message != nullptr)
                {
                    switch (message->GetType())
                    {
                        case (int)GameMessageType::PLAYER_POSITION:
                        {
                            PlayerPositionMessage* posMsg = (PlayerPositionMessage*)message;
                            uint32_t playerId = posMsg->playerId;
                            glm::vec3 position(posMsg->posX, posMsg->posY, posMsg->posZ);

                            // Store player position
                            m_playerPositions[playerId] = position;

                            // Broadcast to all other clients
                            BroadcastPlayerPosition(playerId, position, posMsg->yaw, posMsg->pitch);
                            break;
                        }
                        case (int)GameMessageType::BLOCK_UPDATE:
                        {
                            BlockUpdateMessage* blockMsg = (BlockUpdateMessage*)message;

                            BlockType type = static_cast<BlockType>(blockMsg->blockType);

                            spdlog::debug("Server received block update: ({}, {}, {}) type={} place={}",
                                          blockMsg->blockX, blockMsg->blockY, blockMsg->blockZ,
                                          blockMsg->blockType, blockMsg->isPlacement);

                            // Apply to server world
                            ApplyBlockUpdateInternal(blockMsg->blockX,
                                                     blockMsg->blockY,
                                                     blockMsg->blockZ,
                                                     type,
                                                     blockMsg->isPlacement);

                            // Broadcast to all connected clients
                            const int MAX_PLAYERS = 64;
                            for (int clientIndex = 0; clientIndex < MAX_PLAYERS; ++clientIndex)
                            {
                                if (!m_server->IsClientConnected(clientIndex))
                                    continue;

                                BlockUpdateMessage* outMsg =
                                    (BlockUpdateMessage*)m_server->CreateMessage(clientIndex, (int)GameMessageType::BLOCK_UPDATE);
                                if (!outMsg)
                                    continue;

                                outMsg->blockX      = blockMsg->blockX;
                                outMsg->blockY      = blockMsg->blockY;
                                outMsg->blockZ      = blockMsg->blockZ;
                                outMsg->blockType   = blockMsg->blockType;
                                outMsg->isPlacement = blockMsg->isPlacement;

                                m_server->SendMessage(clientIndex, (int)GameChannel::RELIABLE, outMsg);
                            }

                            break;
                        }
                    }

                    m_server->ReleaseMessage(i, message);
                    message = m_server->ReceiveMessage(i, j);
                }
            }
        }
    }

    void NetworkManager::ProcessClientMessages()
    {
        if (!m_client->IsConnected())
        {
            return;
        }

        for (int j = 0; j < m_config.numChannels; j++)
        {
            yojimbo::Message* message = m_client->ReceiveMessage(j);
            while (message != nullptr)
            {
                switch (message->GetType())
                {
                    case (int)GameMessageType::PLAYER_POSITION:
                    {
                        PlayerPositionMessage* posMsg = (PlayerPositionMessage*)message;
                        // Handle other player positions
                        spdlog::debug("Received player position: id={} pos=({}, {}, {})",
                                     posMsg->playerId, posMsg->posX, posMsg->posY, posMsg->posZ);
                        break;
                    }
                    case (int)GameMessageType::BLOCK_UPDATE:
                    {
                        BlockUpdateMessage* blockMsg = (BlockUpdateMessage*)message;

                        BlockType type = static_cast<BlockType>(blockMsg->blockType);

                        spdlog::debug("Received block update: ({}, {}, {}) type={} place={}",
                                      blockMsg->blockX, blockMsg->blockY, blockMsg->blockZ,
                                      blockMsg->blockType, blockMsg->isPlacement);

                        ApplyBlockUpdateInternal(blockMsg->blockX,
                                                 blockMsg->blockY,
                                                 blockMsg->blockZ,
                                                 type,
                                                 blockMsg->isPlacement);
                        break;
                    }
                }

                m_client->ReleaseMessage(message);
                message = m_client->ReceiveMessage(j);
            }
        }
    }

    void NetworkManager::SendPlayerPosition(const glm::vec3& position, float yaw, float pitch)
    {
        if (m_isServer)
        {
            // Server broadcasts its own position
            BroadcastPlayerPosition(m_localPlayerId, position, yaw, pitch);
        }
        else if (m_client && m_client->IsConnected())
        {
            PlayerPositionMessage* message = (PlayerPositionMessage*)m_client->CreateMessage((int)GameMessageType::PLAYER_POSITION);
            if (message)
            {
                message->playerId = m_localPlayerId;
                message->posX = position.x;
                message->posY = position.y;
                message->posZ = position.z;
                message->yaw = yaw;
                message->pitch = pitch;

                m_client->SendMessage((int)GameChannel::UNRELIABLE, message);
            }
        }
    }

    void NetworkManager::SendBlockUpdate(int x, int y, int z, BlockType type, bool isPlacement)
    {
        if (m_isServer)
        {
            // Server would broadcast to all clients
            spdlog::debug("Server block update: ({}, {}, {})", x, y, z);
        }
        else if (m_client && m_client->IsConnected())
        {
            BlockUpdateMessage* message = (BlockUpdateMessage*)m_client->CreateMessage((int)GameMessageType::BLOCK_UPDATE);
            if (message)
            {
                message->blockX = x;
                message->blockY = y;
                message->blockZ = z;
                message->blockType = static_cast<uint8_t>(type);
                message->isPlacement = isPlacement;

                m_client->SendMessage((int)GameChannel::RELIABLE, message);
            }
        }
    }

    void NetworkManager::BroadcastPlayerPosition(uint32_t playerId, const glm::vec3& position, float yaw, float pitch)
    {
        if (!m_server)
        {
            return;
        }

        const int MAX_PLAYERS = 64;
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (!m_server->IsClientConnected(i))
            {
                continue;
            }

            // Don't send back to the sender
            // (In a real implementation, you'd track which client is which player)

            PlayerPositionMessage* message = (PlayerPositionMessage*)m_server->CreateMessage(i, (int)GameMessageType::PLAYER_POSITION);
            if (message)
            {
                message->playerId = playerId;
                message->posX = position.x;
                message->posY = position.y;
                message->posZ = position.z;
                message->yaw = yaw;
                message->pitch = pitch;

                m_server->SendMessage(i, (int)GameChannel::UNRELIABLE, message);
            }
        }
    }

    void NetworkManager::ApplyBlockUpdateInternal(int x, int y, int z, BlockType type, bool isPlacement)
    {
        if (!m_world || !m_chunkRenderer)
        {
            spdlog::warn("NetworkManager::ApplyBlockUpdateInternal called without world/renderer wired");
            return;
        }

        // Apply to world
        if (isPlacement)
        {
            m_world->SetBlock(x, y, z, type);
        }
        else
        {
            m_world->SetBlock(x, y, z, BlockType::Air);
        }

        // Figure out which chunk this block belongs to
        auto chunkCoords = World::GetChunkCoords(x, z);
        Chunk* chunk = m_world->GetChunk(chunkCoords.first, chunkCoords.second);
        if (!chunk)
        {
            return;
        }

        // Update this chunk's mesh
        m_chunkRenderer->UpdateChunk(chunk, chunkCoords.first, chunkCoords.second, m_world);

        // Optionally: also update adjacent chunks if on a border (same logic as BlockInteraction::MarkChunkForUpdate)
        glm::ivec3 local = World::GetLocalCoords(x, y, z);

        struct Offset { int dx, dz; };
        std::vector<Offset> neighborOffsets;

        const int kChunkSizeX = CHUNK_SIZE_X;
        const int kChunkSizeZ = CHUNK_SIZE_Z;

        if (local.x == 0)                   neighborOffsets.push_back({ -1,  0 });
        if (local.x == kChunkSizeX - 1)     neighborOffsets.push_back({  1,  0 });
        if (local.z == 0)                   neighborOffsets.push_back({  0, -1 });
        if (local.z == kChunkSizeZ - 1)     neighborOffsets.push_back({  0,  1 });

        for (const auto& o : neighborOffsets)
        {
            int adjChunkX = chunkCoords.first  + o.dx;
            int adjChunkZ = chunkCoords.second + o.dz;
            Chunk* adjChunk = m_world->GetChunk(adjChunkX, adjChunkZ);
            if (adjChunk)
            {
                m_chunkRenderer->UpdateChunk(adjChunk, adjChunkX, adjChunkZ, m_world);
            }
        }
    }
}