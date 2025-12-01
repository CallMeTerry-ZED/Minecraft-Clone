/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "Core/Time.h"

namespace MinecraftClone
{
    float Time::s_deltaTime = 0.0f;
    float Time::s_totalTime = 0.0f;

    void Time::Update(float deltaTime)
    {
        s_deltaTime = deltaTime;
        s_totalTime += deltaTime;
    }
}