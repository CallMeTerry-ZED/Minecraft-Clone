/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#pragma once

// NetworkMessages.h includes yojimbo.h, so we can use full types here
#include "Networking/NetworkMessages.h"
#include "World/World.h"
#include "Core/Camera.h"
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>

namespace MinecraftClone
{
    class ChunkRenderer; // forward declaration

    // Connection configuration
    struct GameConnectionConfig : public yojimbo::ClientServerConfig
    {
        GameConnectionConfig()
        {
            numChannels = 2;
            channel[(int)GameChannel::RELIABLE].type = yojimbo::CHANNEL_TYPE_RELIABLE_ORDERED;
            channel[(int)GameChannel::UNRELIABLE].type = yojimbo::CHANNEL_TYPE_UNRELIABLE_UNORDERED;
        }
    };

    // Adapter for callbacks
    class GameAdapter : public yojimbo::Adapter
    {
    public:
        explicit GameAdapter()
        {
        }

        yojimbo::MessageFactory* CreateMessageFactory(yojimbo::Allocator& allocator) override
        {
            return YOJIMBO_NEW(allocator, GameMessageFactory, allocator);
        }
    };

    class NetworkManager
    {
    public:
        NetworkManager();
        ~NetworkManager();

        // Server functions
        bool StartServer(const char* address, int port);
        void StopServer();
        bool IsServerRunning() const { return m_server != nullptr && m_server->IsRunning(); }

        // Client functions
        bool ConnectToServer(const char* address, int port);
        void Disconnect();
        bool IsConnected() const { return m_client != nullptr && m_client->IsConnected(); }
        bool IsConnecting() const { return m_client != nullptr && m_client->IsConnecting(); }

        // Update (call every frame)
        void Update(double time, float deltaTime);

        // Send messages
        void SendPlayerPosition(const glm::vec3& position, float yaw, float pitch);
        void SendBlockUpdate(int x, int y, int z, BlockType type, bool isPlacement);

        // Getters
        bool IsServer() const { return m_isServer; }
        uint32_t GetLocalPlayerId() const { return m_localPlayerId; }

        // World / Renderer wiring
        void SetWorld(World* world) { m_world = world; }
        void SetChunkRenderer(ChunkRenderer* renderer) { m_chunkRenderer = renderer; }

    private:
        // Server
        void UpdateServer(double time, float deltaTime);
        void ProcessServerMessages();
        void BroadcastPlayerPosition(uint32_t playerId, const glm::vec3& position, float yaw, float pitch);

        // Client
        void UpdateClient(double time, float deltaTime);
        void ProcessClientMessages();

        // Shared block-application helper
        void ApplyBlockUpdateInternal(int x, int y, int z, BlockType type, bool isPlacement);

        // Private key for insecure connections (development only)
        static const uint8_t DEFAULT_PRIVATE_KEY[32];

        GameConnectionConfig m_config;
        GameAdapter m_adapter;

        // Server
        std::unique_ptr<yojimbo::Server> m_server;
        std::unordered_map<uint32_t, glm::vec3> m_playerPositions;  // Server tracks all players

        // Client
        std::unique_ptr<yojimbo::Client> m_client;

        bool m_isServer;
        uint32_t m_localPlayerId;
        double m_time;

        // Non-owning pointers into game state
        World* m_world = nullptr;
        ChunkRenderer* m_chunkRenderer = nullptr;
    };
}

#endif