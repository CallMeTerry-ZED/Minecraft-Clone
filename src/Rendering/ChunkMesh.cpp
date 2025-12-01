/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "Rendering/ChunkMesh.h"
#include "Rendering/Shader.h"
#include <spdlog/spdlog.h>

namespace MinecraftClone
{
    ChunkMesh::ChunkMesh() : m_VAO(0), m_VBO(0), m_EBO(0), m_isBuilt(false)
    {
    }

    ChunkMesh::~ChunkMesh()
    {
        Shutdown();
    }

    void ChunkMesh::Clear()
    {
        m_vertices.clear();
        m_indices.clear();
        m_isBuilt = false;
    }

    void ChunkMesh::AddFace(const glm::vec3& position, const glm::vec3& color, const glm::vec3& normal, int faceIndex)
    {
        // Define the 4 vertices of a quad face
        // Face indices: 0=front, 1=back, 2=left, 3=right, 4=top, 5=bottom
        glm::vec3 v0, v1, v2, v3;

        switch (faceIndex)
        {
            case 0: // Front face (+Z)
                v0 = position + glm::vec3(0.0f, 0.0f, 1.0f);
                v1 = position + glm::vec3(1.0f, 0.0f, 1.0f);
                v2 = position + glm::vec3(1.0f, 1.0f, 1.0f);
                v3 = position + glm::vec3(0.0f, 1.0f, 1.0f);
                break;
            case 1: // Back face (-Z)
                v0 = position + glm::vec3(1.0f, 0.0f, 0.0f);
                v1 = position + glm::vec3(0.0f, 0.0f, 0.0f);
                v2 = position + glm::vec3(0.0f, 1.0f, 0.0f);
                v3 = position + glm::vec3(1.0f, 1.0f, 0.0f);
                break;
            case 2: // Left face (-X)
                v0 = position + glm::vec3(0.0f, 0.0f, 0.0f);
                v1 = position + glm::vec3(0.0f, 0.0f, 1.0f);
                v2 = position + glm::vec3(0.0f, 1.0f, 1.0f);
                v3 = position + glm::vec3(0.0f, 1.0f, 0.0f);
                break;
            case 3: // Right face (+X)
                v0 = position + glm::vec3(1.0f, 0.0f, 1.0f);
                v1 = position + glm::vec3(1.0f, 0.0f, 0.0f);
                v2 = position + glm::vec3(1.0f, 1.0f, 0.0f);
                v3 = position + glm::vec3(1.0f, 1.0f, 1.0f);
                break;
            case 4: // Top face (+Y)
                v0 = position + glm::vec3(0.0f, 1.0f, 0.0f);
                v1 = position + glm::vec3(0.0f, 1.0f, 1.0f);
                v2 = position + glm::vec3(1.0f, 1.0f, 1.0f);
                v3 = position + glm::vec3(1.0f, 1.0f, 0.0f);
                break;
            case 5: // Bottom face (-Y)
                v0 = position + glm::vec3(0.0f, 0.0f, 1.0f);
                v1 = position + glm::vec3(0.0f, 0.0f, 0.0f);
                v2 = position + glm::vec3(1.0f, 0.0f, 0.0f);
                v3 = position + glm::vec3(1.0f, 0.0f, 1.0f);
                break;
            default:
                return;
        }

        unsigned int baseIndex = static_cast<unsigned int>(m_vertices.size());

        m_vertices.push_back({v0, color, normal});
        m_vertices.push_back({v1, color, normal});
        m_vertices.push_back({v2, color, normal});
        m_vertices.push_back({v3, color, normal});

        // Add indices for two triangles
        m_indices.push_back(baseIndex + 0);
        m_indices.push_back(baseIndex + 1);
        m_indices.push_back(baseIndex + 2);
        m_indices.push_back(baseIndex + 2);
        m_indices.push_back(baseIndex + 3);
        m_indices.push_back(baseIndex + 0);
    }

    void ChunkMesh::Build()
    {
        if (m_isBuilt)
        {
            Shutdown();
        }

        if (m_vertices.empty())
        {
            return;
        }

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), m_vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);

        // Color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
        glEnableVertexAttribArray(1);

        // Normal attribute
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);

        m_isBuilt = true;
    }

    void ChunkMesh::Render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, Shader* shader)
    {
        if (!m_isBuilt || m_VAO == 0)
        {
            return;
        }

        shader->Use();

        // Model matrix (chunk position)
        glm::mat4 model = glm::mat4(1.0f);
        shader->SetMat4("model", model);
        shader->SetMat4("view", viewMatrix);
        shader->SetMat4("projection", projectionMatrix);

        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        shader->Unuse();
    }

    void ChunkMesh::Shutdown()
    {
        if (m_VAO != 0)
        {
            glDeleteVertexArrays(1, &m_VAO);
            m_VAO = 0;
        }
        if (m_VBO != 0)
        {
            glDeleteBuffers(1, &m_VBO);
            m_VBO = 0;
        }
        if (m_EBO != 0)
        {
            glDeleteBuffers(1, &m_EBO);
            m_EBO = 0;
        }
        m_isBuilt = false;
    }
}