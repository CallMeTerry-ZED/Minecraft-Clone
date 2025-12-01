/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "Rendering/Shader.h"
#include <fstream>
#include <sstream>
#include <spdlog/spdlog.h>

namespace MinecraftClone
{
    Shader::Shader() : m_programID(0)
    {
    }

    Shader::~Shader()
    {
        if (m_programID != 0)
        {
            glDeleteProgram(m_programID);
        }
    }

    bool Shader::LoadFromSource(const std::string& vertexSource, const std::string& fragmentSource)
    {
        GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexSource);
        if (vertexShader == 0)
        {
            return false;
        }

        GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
        if (fragmentShader == 0)
        {
            glDeleteShader(vertexShader);
            return false;
        }

        m_programID = LinkProgram(vertexShader, fragmentShader);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return m_programID != 0;
    }

    bool Shader::LoadFromFile(const std::string& vertexPath, const std::string& fragmentPath)
    {
        std::ifstream vertexFile(vertexPath);
        std::ifstream fragmentFile(fragmentPath);

        if (!vertexFile.is_open())
        {
            spdlog::error("Failed to open vertex shader file: {}", vertexPath);
            return false;
        }

        if (!fragmentFile.is_open())
        {
            spdlog::error("Failed to open fragment shader file: {}", fragmentPath);
            return false;
        }

        std::stringstream vertexStream, fragmentStream;
        vertexStream << vertexFile.rdbuf();
        fragmentStream << fragmentFile.rdbuf();

        vertexFile.close();
        fragmentFile.close();

        return LoadFromSource(vertexStream.str(), fragmentStream.str());
    }

    void Shader::Use() const
    {
        glUseProgram(m_programID);
    }

    void Shader::Unuse() const
    {
        glUseProgram(0);
    }

    GLuint Shader::CompileShader(GLenum type, const std::string& source)
    {
        GLuint shader = glCreateShader(type);
        const char* sourceCStr = source.c_str();
        glShaderSource(shader, 1, &sourceCStr, nullptr);
        glCompileShader(shader);

        GLint success;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            char infoLog[1024];
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            spdlog::error("Shader compilation failed: {}", infoLog);
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    GLuint Shader::LinkProgram(GLuint vertexShader, GLuint fragmentShader)
    {
        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        GLint success;
        glGetProgramiv(program, GL_LINK_STATUS, &success);
        if (!success)
        {
            char infoLog[1024];
            glGetProgramInfoLog(program, 1024, nullptr, infoLog);
            spdlog::error("Shader program linking failed: {}", infoLog);
            glDeleteProgram(program);
            return 0;
        }

        return program;
    }

    GLint Shader::GetUniformLocation(const std::string& name) const
    {
        auto it = m_uniformLocationCache.find(name);
        if (it != m_uniformLocationCache.end())
        {
            return it->second;
        }

        GLint location = glGetUniformLocation(m_programID, name.c_str());
        m_uniformLocationCache[name] = location;
        return location;
    }

    void Shader::SetBool(const std::string& name, bool value) const
    {
        glUniform1i(GetUniformLocation(name), static_cast<int>(value));
    }

    void Shader::SetInt(const std::string& name, int value) const
    {
        glUniform1i(GetUniformLocation(name), value);
    }

    void Shader::SetFloat(const std::string& name, float value) const
    {
        glUniform1f(GetUniformLocation(name), value);
    }

    void Shader::SetVec2(const std::string& name, const glm::vec2& value) const
    {
        glUniform2fv(GetUniformLocation(name), 1, &value[0]);
    }

    void Shader::SetVec3(const std::string& name, const glm::vec3& value) const
    {
        glUniform3fv(GetUniformLocation(name), 1, &value[0]);
    }

    void Shader::SetVec4(const std::string& name, const glm::vec4& value) const
    {
        glUniform4fv(GetUniformLocation(name), 1, &value[0]);
    }

    void Shader::SetMat2(const std::string& name, const glm::mat2& value) const
    {
        glUniformMatrix2fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
    }

    void Shader::SetMat3(const std::string& name, const glm::mat3& value) const
    {
        glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
    }

    void Shader::SetMat4(const std::string& name, const glm::mat4& value) const
    {
        glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &value[0][0]);
    }
}