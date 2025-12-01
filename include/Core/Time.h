/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef TIME_H
#define TIME_H

#pragma once

namespace MinecraftClone
{
    class Time
    {
    public:
        static float GetDeltaTime() { return s_deltaTime; }
        static float GetTotalTime() { return s_totalTime; }
        static void Update(float deltaTime);

    private:
        static float s_deltaTime;
        static float s_totalTime;
    };
}

#endif