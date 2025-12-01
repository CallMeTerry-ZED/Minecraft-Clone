/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "Rendering/TestCube.h"
#include "Rendering/Shader.h"
#include <glm/gtc/matrix_transform.hpp>

namespace MinecraftClone
{
    TestCube::TestCube()
    {
    }

    TestCube::~TestCube()
    {
        Shutdown();
    }

    bool TestCube::Initialize()
    {
        // Create shader with inline source
        const std::string vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 FragColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragColor = aColor;
}
)";

        const std::string fragmentShaderSource = R"(
#version 330 core
out vec4 FragColorOut;

in vec3 FragColor;

void main()
{
    FragColorOut = vec4(FragColor, 1.0);
}
)";

        m_shader = std::make_unique<Shader>();
        if (!m_shader->LoadFromSource(vertexShaderSource, fragmentShaderSource))
        {
            return false;
        }

        // Cube vertices with colors - fixed winding order for proper face culling
        float vertices[] = {
            // positions          // colors
            // Front face
            -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  // red
             0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  // green
             0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  // blue
            -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  // yellow

            // Back face
            -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  // magenta
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  // cyan
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  // white
            -0.5f,  0.5f, -0.5f,  0.5f, 0.5f, 0.5f   // gray
        };

        // Indices with proper winding order (counter-clockwise for front faces)
        unsigned int indices[] = {
            // Front face
            0, 1, 2,  2, 3, 0,
            // Back face
            5, 4, 7,  7, 6, 5,
            // Left face
            4, 0, 3,  3, 7, 4,
            // Right face
            1, 5, 6,  6, 2, 1,
            // Bottom face
            4, 5, 1,  1, 0, 4,
            // Top face
            3, 2, 6,  6, 7, 3
        };

        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);

        glBindVertexArray(m_VAO);

        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // Color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);

        return true;
    }

    void TestCube::Render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
    {
        m_shader->Use();

        // Create model matrix (position the cube further back)
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -3.0f));

        m_shader->SetMat4("model", model);
        m_shader->SetMat4("view", viewMatrix);
        m_shader->SetMat4("projection", projectionMatrix);

        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        m_shader->Unuse();
    }

    void TestCube::Shutdown()
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
        m_shader.reset();
    }
}