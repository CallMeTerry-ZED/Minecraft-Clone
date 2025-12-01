/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef TESTCUBE_H
#define TESTCUBE_H

#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <memory>

namespace MinecraftClone
{
    class Shader;

    class TestCube
    {
    public:
        TestCube();
        ~TestCube();

        bool Initialize();
        void Render(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
        void Shutdown();

    private:
        GLuint m_VAO = 0;
        GLuint m_VBO = 0;
        GLuint m_EBO = 0;
        std::unique_ptr<Shader> m_shader;
    };
}

#endif