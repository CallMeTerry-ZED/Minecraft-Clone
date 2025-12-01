/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "Rendering/Texture.h"
#include <spdlog/spdlog.h>

namespace MinecraftClone
{
    Texture::Texture() : m_textureID(0), m_width(0), m_height(0), m_channels(0)
    {
    }

    Texture::~Texture()
    {
        Shutdown();
    }

    bool Texture::LoadFromFile(const std::string& filepath)
    {
        Shutdown();

        // Flip texture vertically (OpenGL expects bottom-left origin)
        stbi_set_flip_vertically_on_load(true);

        unsigned char* data = stbi_load(filepath.c_str(), &m_width, &m_height, &m_channels, 0);
        if (!data)
        {
            spdlog::error("Failed to load texture: {}", filepath);
            return false;
        }

        glGenTextures(1, &m_textureID);
        glBindTexture(GL_TEXTURE_2D, m_textureID);

        GLenum format = GL_RGB;
        if (m_channels == 1)
            format = GL_RED;
        else if (m_channels == 3)
            format = GL_RGB;
        else if (m_channels == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        stbi_image_free(data);
        glBindTexture(GL_TEXTURE_2D, 0);

        spdlog::info("Loaded texture: {} ({}x{}, {} channels)", filepath, m_width, m_height, m_channels);
        return true;
    }

    void Texture::Bind(GLuint textureUnit) const
    {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(GL_TEXTURE_2D, m_textureID);
    }

    void Texture::Unbind() const
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void Texture::Shutdown()
    {
        if (m_textureID != 0)
        {
            glDeleteTextures(1, &m_textureID);
            m_textureID = 0;
        }
        m_width = 0;
        m_height = 0;
        m_channels = 0;
    }
}