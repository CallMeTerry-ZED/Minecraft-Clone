/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "World/ChunkRenderer.h"
#include "World/ChunkMeshGenerator.h"
#include "World/World.h"
#include "Rendering/BlockTextureRegistry.h"
#include <spdlog/spdlog.h>
#include <set>

namespace MinecraftClone
{
    ChunkRenderer::ChunkRenderer() : m_textureArrayID(0)
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

        // Load all unique textures
        std::set<std::string> uniquePaths;
        std::vector<std::string> orderedPaths; // Keep order for texture array
        for (int type = 0; type < static_cast<int>(BlockType::Count); type++)
        {
            for (int face = 0; face < 6; face++)
            {
                std::string path = BlockTextureRegistry::GetTexturePath(
                    static_cast<BlockType>(type),
                    static_cast<BlockFace>(face)
                );
                if (uniquePaths.find(path) == uniquePaths.end())
                {
                    uniquePaths.insert(path);
                    orderedPaths.push_back(path);
                }
            }
        }

        // Load each unique texture
        for (const auto& path : orderedPaths)
        {
            if (m_textures.find(path) == m_textures.end())
            {
                auto texture = std::make_unique<Texture>();
                if (texture->LoadFromFile(path))
                {
                    m_textures[path] = std::move(texture);
                }
            }
        }

        // Create mapping from block+face to texture path, then to index
        std::unordered_map<std::string, uint32_t> pathToIndex;
        uint32_t index = 0;
        for (const auto& path : orderedPaths)
        {
            pathToIndex[path] = index++;
        }

        // Map block type + face to texture index
        for (int type = 0; type < static_cast<int>(BlockType::Count); type++)
        {
            for (int face = 0; face < 6; face++)
            {
                std::string path = BlockTextureRegistry::GetTexturePath(
                    static_cast<BlockType>(type),
                    static_cast<BlockFace>(face)
                );
                uint32_t key = (static_cast<uint32_t>(type) << 8) | static_cast<uint32_t>(face);
                if (pathToIndex.find(path) != pathToIndex.end())
                {
                    m_textureIndices[key] = pathToIndex[path];
                }
            }
        }

        spdlog::info("Loaded {} unique textures", m_textures.size());

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

        // Bind a default texture (use stone texture as it's common)
        // For proper per-face textures, we'd need texture arrays or separate draw calls
        // For now, bind a visible texture instead of the first one in the map
        std::string defaultTexturePath = BlockTextureRegistry::GetTexturePath(BlockType::Stone, BlockFace::Front);
        if (m_textures.find(defaultTexturePath) != m_textures.end())
        {
            m_textures[defaultTexturePath]->Bind(0);
        }
        else if (!m_textures.empty())
        {
            // Fallback to first available texture
            m_textures.begin()->second->Bind(0);
        }
        m_shader->SetInt("blockTexture", 0);

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
        
        // Clean up textures
        for (auto& [path, texture] : m_textures)
        {
            if (texture)
            {
                texture->Shutdown();
            }
        }
        m_textures.clear();
        
        if (m_textureArrayID != 0)
        {
            glDeleteTextures(1, &m_textureArrayID);
            m_textureArrayID = 0;
        }
        
        m_shader.reset();
    }
}