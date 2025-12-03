/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "Audio/SoundManager.h"
#include <spdlog/spdlog.h>

namespace MinecraftClone
{
    SoundManager& SoundManager::Instance()
    {
        static SoundManager instance;
        return instance;
    }

    SoundManager::SoundManager()
        : m_masterVolume(1.0f)
        , m_initialized(false)
    {
    }

    SoundManager::~SoundManager()
    {
        Shutdown();
    }

    bool SoundManager::Initialize()
    {
        if (m_initialized)
        {
            return true;
        }

        // Initialize SoLoud
        SoLoud::result result = m_soloud.init();
        if (result != SoLoud::SO_NO_ERROR)
        {
            spdlog::error("Failed to initialize SoLoud: {}", result);
            return false;
        }

        // Enable 3D audio
        m_soloud.set3dSoundSpeed(343.3f); // Speed of sound in air (m/s)
        m_soloud.set3dListenerParameters(0, 0, 0,  // Position
                                         0, 0, -1,  // Forward (looking along -Z)
                                         0, 1, 0);  // Up

        // Load sounds (using placeholder paths for now - you'll need actual sound files)
        // For now, we'll just create empty wav objects to prevent crashes
        // TODO: Add actual sound files to assets/sounds/
        
        spdlog::info("SoundManager initialized");
        m_initialized = true;
        return true;
    }

    void SoundManager::Shutdown()
    {
        if (!m_initialized)
        {
            return;
        }

        // Stop all sounds
        m_soloud.stopAll();
        
        // Clear all loaded sounds
        m_sounds.clear();
        
        // Cleanup SoLoud
        m_soloud.deinit();
        
        m_initialized = false;
        spdlog::info("SoundManager shut down");
    }

    bool SoundManager::LoadSound(SoundType type, const std::string& filepath)
    {
        auto wav = std::make_unique<SoLoud::Wav>();
        
        SoLoud::result result = wav->load(filepath.c_str());
        if (result != SoLoud::SO_NO_ERROR)
        {
            spdlog::warn("Failed to load sound: {} (error: {})", filepath, result);
            return false;
        }

        m_sounds[type] = std::move(wav);
        spdlog::info("Loaded sound: {}", filepath);
        return true;
    }

    void SoundManager::PlaySound(SoundType type, float volume)
    {
        if (!m_initialized)
        {
            return;
        }

        auto it = m_sounds.find(type);
        if (it != m_sounds.end() && it->second)
        {
            m_soloud.play(*it->second, volume * m_masterVolume);
        }
    }

    void SoundManager::PlaySound3D(SoundType type, float x, float y, float z, float volume)
    {
        if (!m_initialized)
        {
            return;
        }

        auto it = m_sounds.find(type);
        if (it != m_sounds.end() && it->second)
        {
            m_soloud.play3d(*it->second, x, y, z, 0, 0, 0, volume * m_masterVolume);
        }
    }

    void SoundManager::SetListenerPosition(float x, float y, float z)
    {
        if (!m_initialized)
        {
            return;
        }

        // Keep current orientation, just update position
        m_soloud.set3dListenerPosition(x, y, z);
    }

    void SoundManager::SetListenerOrientation(float forwardX, float forwardY, float forwardZ,
                                              float upX, float upY, float upZ)
    {
        if (!m_initialized)
        {
            return;
        }

        m_soloud.set3dListenerAt(forwardX, forwardY, forwardZ);
        m_soloud.set3dListenerUp(upX, upY, upZ);
    }

    void SoundManager::Update()
    {
        if (!m_initialized)
        {
            return;
        }

        // Update 3D audio calculations
        m_soloud.update3dAudio();
    }

    void SoundManager::SetMasterVolume(float volume)
    {
        m_masterVolume = std::max(0.0f, std::min(1.0f, volume));
        m_soloud.setGlobalVolume(m_masterVolume);
    }
}

