/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef CHUNKMESH_H
#define CHUNKMESH_H

#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>
#include <memory>

namespace MinecraftClone
{
    class Shader;

    struct Vertex
    {
        glm::vec3 position;
        glm::vec2 texCoord;
        glm::vec3 normal;
    };

    class ChunkMesh
    {
    public:
        ChunkMesh();
        ~ChunkMesh();

        void Clear();
        void AddFace(const glm::vec3& position, const glm::vec2& texCoord0, const glm::vec2& texCoord1,
            const glm::vec2& texCoord2, const glm::vec2& texCoord3, const glm::vec3& normal, int faceIndex);
        void AddQuad(const glm::vec3& position, float width, float height, const glm::vec3& normal, int faceIndex);
        void Build();
        void Render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix, Shader* shader);
        void Shutdown();

        bool IsEmpty() const { return m_vertices.empty(); }
        size_t GetVertexCount() const { return m_vertices.size(); }
        size_t GetIndexCount() const { return m_indices.size(); }

    private:
        std::vector<Vertex> m_vertices;
        std::vector<unsigned int> m_indices;

        GLuint m_VAO;
        GLuint m_VBO;
        GLuint m_EBO;

        bool m_isBuilt;
    };
}

#endif