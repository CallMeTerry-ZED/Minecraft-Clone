/*
* © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef TEXTURE_H
#define TEXTURE_H

#pragma once

#include <glad/gl.h>
#include <string>

namespace MinecraftClone
{
    class Texture
    {
    public:
        Texture();
        ~Texture();

        bool LoadFromFile(const std::string& filepath);
        void Bind(GLuint textureUnit = 0) const;
        void Unbind() const;

        GLuint GetID() const { return m_textureID; }
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }

        void Shutdown();

    private:
        GLuint m_textureID;
        int m_width;
        int m_height;
        int m_channels;
    };
}

#endif