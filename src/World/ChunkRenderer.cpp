/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "World/ChunkRenderer.h"
#include "World/ChunkMeshGenerator.h"
#include "World/World.h"
#include "Rendering/BlockTextureRegistry.h"
#include "Rendering/Frustum.h"
#include <spdlog/spdlog.h>
#include <set>

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
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

        const std::string fragmentShaderSource = R"(
#version 330 core
out vec4 FragColorOut;

in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D blockTexture;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

void main()
{
    vec4 texColor = texture(blockTexture, TexCoord);

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

    vec3 result = (ambient + diffuse + specular) * texColor.rgb;
    FragColorOut = vec4(result, texColor.a);
}
)";

        m_shader = std::make_unique<Shader>();
        if (!m_shader->LoadFromSource(vertexShaderSource, fragmentShaderSource))
        {
            spdlog::error("Failed to create chunk shader!");
            return false;
        }

        // Initialize texture registry
        BlockTextureRegistry::Initialize();

        // Load texture atlas
        std::string atlasPath = "assets/textures/block_atlas.png";
        m_atlasTexture = std::make_unique<Texture>();
        if (!m_atlasTexture->LoadFromFile(atlasPath))
        {
            spdlog::error("Failed to load texture atlas: {}", atlasPath);
            return false;
        }

        spdlog::info("Loaded texture atlas: {}", atlasPath);

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

    void ChunkRenderer::SetChunkMesh(int chunkX, int chunkZ, std::unique_ptr<ChunkMesh> mesh)
    {
        auto key = std::make_pair(chunkX, chunkZ);
        m_chunkMeshes[key] = std::move(mesh);
    }

    void ChunkRenderer::RenderChunks(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
    {
        if (!m_shader)
        {
            return;
        }

        // Extract frustum planes from view-projection matrix for culling
        glm::mat4 viewProjection = projectionMatrix * viewMatrix;
        m_frustum.ExtractPlanes(viewProjection);

        m_shader->Use();

        // Bind texture atlas
        if (m_atlasTexture)
        {
            m_atlasTexture->Bind(0);
        }
        m_shader->SetInt("blockTexture", 0);

        // Set up lighting (simple directional light)
        glm::vec3 lightPos = glm::vec3(100.0f, 100.0f, 100.0f);
        glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
        glm::vec3 viewPos = glm::vec3(0.0f, 0.0f, 0.0f); // Will be set from camera

        m_shader->SetVec3("lightPos", lightPos);
        m_shader->SetVec3("lightColor", lightColor);
        m_shader->SetVec3("viewPos", viewPos);

        int chunksRendered = 0;
        int chunksCulled = 0;

        for (auto& [coord, mesh] : m_chunkMeshes)
        {
            if (mesh && !mesh->IsEmpty())
            {
                // Calculate chunk bounding box in world space
                int chunkX = coord.first;
                int chunkZ = coord.second;
                
                // Chunk spans from (chunkX * 16, 0, chunkZ * 16) to ((chunkX + 1) * 16, 256, (chunkZ + 1) * 16)
                glm::vec3 chunkMin(
                    static_cast<float>(chunkX * CHUNK_SIZE_X),
                    0.0f,
                    static_cast<float>(chunkZ * CHUNK_SIZE_Z)
                );
                glm::vec3 chunkMax(
                    static_cast<float>((chunkX + 1) * CHUNK_SIZE_X),
                    static_cast<float>(CHUNK_SIZE_Y),
                    static_cast<float>((chunkZ + 1) * CHUNK_SIZE_Z)
                );

                // Test if chunk is visible in frustum
                if (m_frustum.IsAABBVisible(chunkMin, chunkMax))
                {
                    mesh->Render(viewMatrix, projectionMatrix, m_shader.get());
                    chunksRendered++;
                }
                else
                {
                    chunksCulled++;
                }
            }
        }

        // Log culling stats occasionally (every 60 frames or so)
        static int frameCount = 0;
        if (++frameCount % 60 == 0)
        {
            int totalChunks = chunksRendered + chunksCulled;
            if (totalChunks > 0)
            {
                float cullRatio = (static_cast<float>(chunksCulled) / static_cast<float>(totalChunks)) * 100.0f;
                spdlog::info("Frustum culling: {} rendered, {} culled ({:.1f}% culled)", 
                    chunksRendered, chunksCulled, cullRatio);
            }
        }

        m_shader->Unuse();
        
        // Unbind texture
        glBindTexture(GL_TEXTURE_2D, 0);
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
        
        // Clean up atlas texture
        if (m_atlasTexture)
        {
            m_atlasTexture->Shutdown();
            m_atlasTexture.reset();
        }
        
        m_shader.reset();
    }
}