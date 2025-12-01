/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "World/ChunkRenderer.h"
#include "World/ChunkMeshGenerator.h"
#include "World/World.h"
#include <spdlog/spdlog.h>

namespace MinecraftClone
{
    ChunkRenderer::ChunkRenderer()
    {
    }

    ChunkRenderer::~ChunkRenderer()
    {
        Shutdown();
    }

    bool ChunkRenderer::Initialize()
    {
        const std::string vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;

out vec3 FragColor;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
    FragColor = aColor;
}
)";

        const std::string fragmentShaderSource = R"(
#version 330 core
out vec4 FragColorOut;

in vec3 FragColor;
in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

void main()
{
    // Ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    float specularStrength = 0.1;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * FragColor;
    FragColorOut = vec4(result, 1.0);
}
)";

        m_shader = std::make_unique<Shader>();
        if (!m_shader->LoadFromSource(vertexShaderSource, fragmentShaderSource))
        {
            spdlog::error("Failed to create chunk shader!");
            return false;
        }

        return true;
    }

    void ChunkRenderer::UpdateChunk(Chunk* chunk, int chunkX, int chunkZ, World* world)
    {
        if (!chunk)
        {
            return;
        }

        auto key = std::make_pair(chunkX, chunkZ);
        auto mesh = ChunkMeshGenerator::GenerateMesh(chunk, chunkX, chunkZ, world);
        m_chunkMeshes[key] = std::move(mesh);
    }

    void ChunkRenderer::RenderChunks(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
    {
        if (!m_shader)
        {
            return;
        }

        m_shader->Use();

        // Set up lighting (simple directional light)
        glm::vec3 lightPos = glm::vec3(100.0f, 100.0f, 100.0f);
        glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 viewPos = glm::vec3(0.0f, 0.0f, 0.0f); // Will be set from camera

        m_shader->SetVec3("lightPos", lightPos);
        m_shader->SetVec3("lightColor", lightColor);
        m_shader->SetVec3("viewPos", viewPos);

        for (auto& [coord, mesh] : m_chunkMeshes)
        {
            if (mesh && !mesh->IsEmpty())
            {
                mesh->Render(viewMatrix, projectionMatrix, m_shader.get());
            }
        }

        m_shader->Unuse();
    }

    void ChunkRenderer::UnloadChunk(int chunkX, int chunkZ)
    {
        auto key = std::make_pair(chunkX, chunkZ);
        auto it = m_chunkMeshes.find(key);
        if (it != m_chunkMeshes.end())
        {
            it->second->Shutdown();
            m_chunkMeshes.erase(it);
        }
    }

    void ChunkRenderer::Shutdown()
    {
        for (auto& [coord, mesh] : m_chunkMeshes)
        {
            if (mesh)
            {
                mesh->Shutdown();
            }
        }
        m_chunkMeshes.clear();
        m_shader.reset();
    }
}