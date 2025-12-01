/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include <spdlog/spdlog.h>
#include "Rendering/RemotePlayerRenderer.h"
#include "Networking/NetworkManager.h"   // for RemotePlayer
#include <glad/gl.h>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

namespace MinecraftClone
{
    namespace
    {
        // Simple HSV (0..1) to RGB helper
        glm::vec3 HsvToRgb(float h, float s, float v)
        {
            h = fmodf(h, 1.0f);
            if (h < 0.0f) h += 1.0f;

            float c = v * s;
            float x = c * (1.0f - fabsf(fmodf(h * 6.0f, 2.0f) - 1.0f));
            float m = v - c;

            float r, g, b;
            if      (h < 1.0f / 6.0f) { r = c; g = x; b = 0.0f; }
            else if (h < 2.0f / 6.0f) { r = x; g = c; b = 0.0f; }
            else if (h < 3.0f / 6.0f) { r = 0.0f; g = c; b = x; }
            else if (h < 4.0f / 6.0f) { r = 0.0f; g = x; b = c; }
            else if (h < 5.0f / 6.0f) { r = x; g = 0.0f; b = c; }
            else                      { r = c; g = 0.0f; b = x; }

            return glm::vec3(r + m, g + m, b + m);
        }

        // Deterministic rainbow color based on playerId
        glm::vec3 ColorFromPlayerId(uint32_t id)
        {
            // Use golden-ratio hash to spread hues nicely
            float h = fmodf(id * 0.61803398875f, 1.0f);
            float s = 0.9f;
            float v = 0.9f;
            return HsvToRgb(h, s, v);
        }
    }

    RemotePlayerRenderer::RemotePlayerRenderer() = default;

    RemotePlayerRenderer::~RemotePlayerRenderer()
    {
        Shutdown();
    }

    bool RemotePlayerRenderer::Initialize()
    {
        // Minimal shader: solid-colored cubes
        const char* vertexSrc = R"(
            #version 460 core
            layout(location = 0) in vec3 aPos;

            uniform mat4 u_Model;
            uniform mat4 u_View;
            uniform mat4 u_Projection;

            void main()
            {
                gl_Position = u_Projection * u_View * u_Model * vec4(aPos, 1.0);
            }
        )";

        const char* fragmentSrc = R"(
            #version 460 core
            out vec4 FragColor;

            uniform vec3 u_Color;

            void main()
            {
                FragColor = vec4(u_Color, 1.0);
            }
        )";

        m_shader = std::make_unique<Shader>();
        if (!m_shader->LoadFromSource(vertexSrc, fragmentSrc))
        {
            spdlog::error("Failed to load RemotePlayerRenderer shader from source");
            return false;
        }

        CreateCubeMesh();
        return true;
    }

    void RemotePlayerRenderer::CreateCubeMesh()
    {
        // Simple unit cube centered at origin (like TestCube but without colors/UVs)
        float vertices[] = {
            // positions (36 vertices, 12 triangles)
            // Front face
            -0.5f, -0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,

            // Back face
            -0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,

            // Left face
            -0.5f, -0.5f, -0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,

            // Right face
             0.5f, -0.5f, -0.5f,
             0.5f,  0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f,  0.5f,  0.5f,

            // Top face
            -0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
             0.5f,  0.5f,  0.5f,
             0.5f,  0.5f, -0.5f,

            // Bottom face
            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f, -0.5f,  0.5f,
        };

        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void RemotePlayerRenderer::Render(const std::unordered_map<uint32_t, RemotePlayer>& players,
                                      const glm::mat4& view,
                                      const glm::mat4& projection)
    {
        if (players.empty() || !m_shader || m_vao == 0)
            return;

        m_shader->Use();
        m_shader->SetMat4("u_View", view);
        m_shader->SetMat4("u_Projection", projection);

        glBindVertexArray(m_vao);

        for (const auto& [id, rp] : players)
        {
            glm::vec3 color = ColorFromPlayerId(id);
            m_shader->SetVec3("u_Color", color);

            glm::mat4 model(1.0f);
            model = glm::translate(model, rp.position + glm::vec3(0.0f, 0.5f, 0.0f));
            model = glm::scale(model, glm::vec3(0.8f, 1.8f, 0.8f));

            m_shader->SetMat4("u_Model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glBindVertexArray(0);
        m_shader->Unuse();
    }

    void RemotePlayerRenderer::Shutdown()
    {
        if (m_vbo)
        {
            glDeleteBuffers(1, &m_vbo);
            m_vbo = 0;
        }
        if (m_vao)
        {
            glDeleteVertexArrays(1, &m_vao);
            m_vao = 0;
        }
        if (m_shader)
        {
            m_shader.reset();
        }
    }
}