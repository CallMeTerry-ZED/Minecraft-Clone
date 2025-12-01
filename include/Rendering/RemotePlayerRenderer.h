/*
* © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef REMOTEPLAYERRENDERER_H
#define REMOTEPLAYERRENDERER_H

#pragma once

#include "Rendering/Shader.h"
#include <glm/glm.hpp>
#include <unordered_map>
#include <cstdint>
#include <memory>

namespace MinecraftClone
{
    struct RemotePlayer; // forward declare from NetworkManager

    class RemotePlayerRenderer
    {
    public:
        RemotePlayerRenderer();
        ~RemotePlayerRenderer();

        bool Initialize();
        void Render(const std::unordered_map<uint32_t, RemotePlayer>& players,
                    const glm::mat4& view,
                    const glm::mat4& projection);
        void Shutdown();

    private:
        void CreateCubeMesh();

        unsigned int m_vao = 0;
        unsigned int m_vbo = 0;
        std::unique_ptr<Shader> m_shader;
    };
}

#endif