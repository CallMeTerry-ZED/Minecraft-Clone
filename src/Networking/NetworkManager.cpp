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
            m_clientChunksSent.clear();
            m_clientChunkQueue.clear();  // Clear queues
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
        (void)deltaTime;

        if (!m_server->IsRunning())
        {
            return;
        }

        m_server->AdvanceTime(time);
        m_server->ReceivePackets();

        // Check for new client connections
        const int MAX_PLAYERS = 64;
        for (int i = 0; i < MAX_PLAYERS; i++)
        {
            if (m_server->IsClientConnected(i))
            {
                // Check if this is a new client (not in our tracking map)
                if (m_clientChunksSent.find(i) == m_clientChunksSent.end())
                {
                    OnClientConnected(i);
                }

                // Process chunk queue for this client
                ProcessChunkQueue(i);
            }
        }

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

                            // Store player position on server
                            m_playerPositions[playerId] = position;

                            // Also update remote-player map so the host can render clients
                            RemotePlayer rp;
                            rp.position = position;
                            rp.yaw      = posMsg->yaw;
                            rp.pitch    = posMsg->pitch;
                            m_remotePlayers[playerId] = rp;

                            // Broadcast to all other clients
                            BroadcastPlayerPosition(playerId, position, posMsg->yaw, posMsg->pitch);
                            break;
                        }
                        case (int)GameMessageType::BLOCK_UPDATE:
                        {
                            BlockUpdateMessage* blockMsg = (BlockUpdateMessage*)message;

                            BlockType type = static_cast<BlockType>(blockMsg->blockType);

                            spdlog::info("Server received block update: ({}, {}, {}) type={} place={}",
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

                        // Ignore our own echo (if any)
                        if (posMsg->playerId != m_localPlayerId)
                        {
                            RemotePlayer rp;
                            rp.position = glm::vec3(posMsg->posX, posMsg->posY, posMsg->posZ);
                            rp.yaw      = posMsg->yaw;
                            rp.pitch    = posMsg->pitch;

                            m_remotePlayers[posMsg->playerId] = rp;
                        }

                        spdlog::info("Received player position: id={} pos=({}, {}, {})",
                                      posMsg->playerId, posMsg->posX, posMsg->posY, posMsg->posZ);
                        break;
                    }
                    case (int)GameMessageType::BLOCK_UPDATE:
                    {
                        BlockUpdateMessage* blockMsg = (BlockUpdateMessage*)message;

                        BlockType type = static_cast<BlockType>(blockMsg->blockType);

                        spdlog::info("Received block update: ({}, {}, {}) type={} place={}",
                                      blockMsg->blockX, blockMsg->blockY, blockMsg->blockZ,
                                      blockMsg->blockType, blockMsg->isPlacement);

                        ApplyBlockUpdateInternal(blockMsg->blockX,
                                                 blockMsg->blockY,
                                                 blockMsg->blockZ,
                                                 type,
                                                 blockMsg->isPlacement);
                        break;
                    }
                    case (int)GameMessageType::CHUNK_DATA:
                    {
                        ChunkSliceMessage* sliceMsg = (ChunkSliceMessage*)message;

                        if (!m_world || !m_chunkRenderer)
                        {
                            spdlog::warn("Received CHUNK_DATA but world/renderer not wired!");
                            break;
                        }

                        spdlog::info("Received chunk slice: ({}, {}) sliceY={}",
                                     sliceMsg->chunkX, sliceMsg->chunkZ, sliceMsg->sliceY);

                        // Get or create chunk
                        Chunk* chunk = m_world->GetOrCreateChunk(sliceMsg->chunkX, sliceMsg->chunkZ);

                        // Apply this slice's block data
                        int yStart = sliceMsg->sliceY * 16;
                        for (int y = 0; y < 16; y++)
                        {
                            int worldY = yStart + y;
                            for (int z = 0; z < CHUNK_SIZE_Z; z++)
                            {
                                for (int x = 0; x < CHUNK_SIZE_X; x++)
                                {
                                    int index = y * (CHUNK_SIZE_X * CHUNK_SIZE_Z) + z * CHUNK_SIZE_X + x;
                                    BlockType type = static_cast<BlockType>(sliceMsg->blockData[index]);
                                    chunk->SetBlock(x, worldY, z, type);
                                }
                            }
                        }

                        // Track received slices
                        auto chunkKey = std::make_pair(sliceMsg->chunkX, sliceMsg->chunkZ);
                        m_clientChunkSlicesReceived[chunkKey].set(sliceMsg->sliceY, true);

                        // If all 16 slices received, update mesh
                        if (m_clientChunkSlicesReceived[chunkKey].all())
                        {
                            m_chunkRenderer->UpdateChunk(chunk, sliceMsg->chunkX, sliceMsg->chunkZ, m_world);
                            m_clientChunkSlicesReceived.erase(chunkKey);  // Clean up
                            spdlog::info("Completed chunk ({}, {}) - mesh updated", sliceMsg->chunkX, sliceMsg->chunkZ);
                        }

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
        if (m_isServer && m_server)
        {
            // Server/host: world has already been updated locally by BlockInteraction.
            // Just broadcast to all connected clients so they mirror the change.
            spdlog::info("Server broadcasting block update: ({}, {}, {}) type={} place={}",
                          x, y, z, static_cast<int>(type), isPlacement);

            const int MAX_PLAYERS = 64;
            for (int clientIndex = 0; clientIndex < MAX_PLAYERS; ++clientIndex)
            {
                if (!m_server->IsClientConnected(clientIndex))
                    continue;

                BlockUpdateMessage* outMsg =
                    (BlockUpdateMessage*)m_server->CreateMessage(clientIndex, (int)GameMessageType::BLOCK_UPDATE);
                if (!outMsg)
                    continue;

                outMsg->blockX      = x;
                outMsg->blockY      = y;
                outMsg->blockZ      = z;
                outMsg->blockType   = static_cast<uint8_t>(type);
                outMsg->isPlacement = isPlacement;

                m_server->SendMessage(clientIndex, (int)GameChannel::RELIABLE, outMsg);
            }
        }
        else if (m_client && m_client->IsConnected())
        {
            BlockUpdateMessage* message =
                (BlockUpdateMessage*)m_client->CreateMessage((int)GameMessageType::BLOCK_UPDATE);
            if (message)
            {
                message->blockX      = x;
                message->blockY      = y;
                message->blockZ      = z;
                message->blockType   = static_cast<uint8_t>(type);
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

    void NetworkManager::ProcessChunkQueue(int clientIndex)
    {
        if (!m_server || !m_server->IsClientConnected(clientIndex) || !m_world)
        {
            return;
        }

        auto& queue = m_clientChunkQueue[clientIndex];
        if (queue.empty())
        {
            return;
        }

        // Send up to 4 slices per frame to avoid flooding the channel
        const int MAX_SLICES_PER_FRAME = 4;
        int sent = 0;

        while (!queue.empty() && sent < MAX_SLICES_PER_FRAME)
        {
            PendingChunkSlice pending = queue.front();
            queue.pop();

            Chunk* chunk = m_world->GetChunk(pending.chunkX, pending.chunkZ);
            if (!chunk)
            {
                continue;  // Chunk doesn't exist, skip
            }

            ChunkSliceMessage* msg = (ChunkSliceMessage*)m_server->CreateMessage(clientIndex, (int)GameMessageType::CHUNK_DATA);
            if (!msg)
            {
                // Channel might be full, put back and try next frame
                queue.push(pending);
                break;
            }

            msg->chunkX = pending.chunkX;
            msg->chunkZ = pending.chunkZ;
            msg->sliceY = pending.sliceY;

            // Copy this slice's block data
            int yStart = pending.sliceY * 16;
            for (int y = 0; y < 16; y++)
            {
                int worldY = yStart + y;
                for (int z = 0; z < CHUNK_SIZE_Z; z++)
                {
                    for (int x = 0; x < CHUNK_SIZE_X; x++)
                    {
                        const Block& block = chunk->GetBlock(x, worldY, z);
                        int index = y * (CHUNK_SIZE_X * CHUNK_SIZE_Z) + z * CHUNK_SIZE_X + x;
                        msg->blockData[index] = static_cast<uint8_t>(block.GetType());
                    }
                }
            }

            if (!m_server->CanSendMessage(clientIndex, (int)GameChannel::RELIABLE))
            {
                // Channel full, put back and try next frame
                m_server->ReleaseMessage(clientIndex, msg);
                queue.push(pending);
                break;
            }

            m_server->SendMessage(clientIndex, (int)GameChannel::RELIABLE, msg);
            sent++;
        }

        if (sent > 0)
        {
            spdlog::info("Sent {} chunk slices to client {} ({} remaining in queue)",
                         sent, clientIndex, static_cast<int>(queue.size()));
        }
    }

    void NetworkManager::SendChunkToClient(int clientIndex, int chunkX, int chunkZ)
    {
        if (!m_server || !m_server->IsClientConnected(clientIndex) || !m_world)
        {
            return;
        }

        // Check if we already sent this chunk
        auto& sentChunks = m_clientChunksSent[clientIndex];
        auto chunkKey = std::make_pair(chunkX, chunkZ);
        if (sentChunks.find(chunkKey) != sentChunks.end())
        {
            return;  // Already sent
        }

        Chunk* chunk = m_world->GetChunk(chunkX, chunkZ);
        if (!chunk)
        {
            return;
        }

        // Queue all 16 slices instead of sending immediately
        auto& queue = m_clientChunkQueue[clientIndex];
        for (uint8_t sliceY = 0; sliceY < 16; sliceY++)
        {
            PendingChunkSlice pending;
            pending.chunkX = chunkX;
            pending.chunkZ = chunkZ;
            pending.sliceY = sliceY;
            queue.push(pending);
        }

        sentChunks.insert(chunkKey);
        spdlog::info("Queued chunk ({}, {}) for client {} (16 slices)", chunkX, chunkZ, clientIndex);
    }

    void NetworkManager::SendChunksAroundPosition(int clientIndex, const glm::vec3& position, int radius)
    {
        auto chunkCoords = World::GetChunkCoords(static_cast<int>(position.x), static_cast<int>(position.z));
        int centerChunkX = chunkCoords.first;
        int centerChunkZ = chunkCoords.second;

        for (int dx = -radius; dx <= radius; dx++)
        {
            for (int dz = -radius; dz <= radius; dz++)
            {
                int chunkX = centerChunkX + dx;
                int chunkZ = centerChunkZ + dz;
                SendChunkToClient(clientIndex, chunkX, chunkZ);
            }
        }
    }

    void NetworkManager::OnClientConnected(int clientIndex)
    {
        spdlog::info("Client {} connected, sending initial chunks", clientIndex);

        // Send chunks around spawn (0, 0, 100)
        glm::vec3 spawnPos(0.0f, 100.0f, 0.0f);
        SendChunksAroundPosition(clientIndex, spawnPos, 5);  // 5 chunk radius
    }
}