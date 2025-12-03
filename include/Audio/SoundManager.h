/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#pragma once

#include <soloud.h>
#include <soloud_wav.h>
#include <memory>
#include <unordered_map>
#include <string>

namespace MinecraftClone
{
    enum class SoundType
    {
        BlockBreak,
        BlockPlace,
        FootstepGrass,
        FootstepStone,
        FootstepWood,
        FootstepSand,
        Jump,
        Ambient
    };

    class SoundManager
    {
    public:
        static SoundManager& Instance();
        
        bool Initialize();
        void Shutdown();
        
        void PlaySound(SoundType type, float volume = 1.0f);
        void PlaySound3D(SoundType type, float x, float y, float z, float volume = 1.0f);
        void SetListenerPosition(float x, float y, float z);
        void SetListenerOrientation(float forwardX, float forwardY, float forwardZ,
                                   float upX, float upY, float upZ);
        
        void Update(); // Call each frame
        void SetMasterVolume(float volume);
        float GetMasterVolume() const { return m_masterVolume; }

    private:
        SoundManager();
        ~SoundManager();
        SoundManager(const SoundManager&) = delete;
        SoundManager& operator=(const SoundManager&) = delete;
        
        bool LoadSound(SoundType type, const std::string& filepath);
        
        SoLoud::Soloud m_soloud;
        std::unordered_map<SoundType, std::unique_ptr<SoLoud::Wav>> m_sounds;
        float m_masterVolume;
        bool m_initialized;
    };
}

#endif

