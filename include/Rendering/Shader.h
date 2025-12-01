/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef SHADER_H
#define SHADER_H

#pragma once

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

namespace MinecraftClone
{
    class Shader
    {
    public:
        Shader();
        ~Shader();

        bool LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource);
        bool LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath);
        void Use() const;
        void Unuse() const;

        // Uniform setters
        void SetBool(const std::string& name, bool value) const;
        void SetInt(const std::string& name, int value) const;
        void SetFloat(const std::string& name, float value) const;
        void SetVec2(const std::string& name, const glm::vec2& value) const;
        void SetVec3(const std::string& name, const glm::vec3& value) const;
        void SetVec4(const std::string& name, const glm::vec4& value) const;
        void SetMat2(const std::string& name, const glm::mat2& value) const;
        void SetMat3(const std::string& name, const glm::mat3& value) const;
        void SetMat4(const std::string& name, const glm::mat4& value) const;

        GLuint GetID() const { return m_programID; }

    private:
        GLuint CompileShader(GLenum type, const std::string& source);
        GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);
        GLint GetUniformLocation(const std::string& name) const;

        GLuint m_programID;
        mutable std::unordered_map<std::string, GLint> m_uniformLocationCache;
    };
}

#endif