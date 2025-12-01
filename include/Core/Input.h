/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef INPUT_H
#define INPUT_H

#pragma once

#include <GLFW/glfw3.h>
#include <unordered_map>

namespace MinecraftClone
{
    class Input
    {
    public:
        static void Initialize(GLFWwindow* window);
        static void Update();
        static void Shutdown();

        // Keyboard
        static bool IsKeyPressed(int key);
        static bool IsKeyDown(int key);
        static bool IsKeyReleased(int key);

        // Mouse
        static bool IsMouseButtonPressed(int button);
        static bool IsMouseButtonDown(int button);
        static bool IsMouseButtonReleased(int button);
        static void GetMousePosition(float& x, float& y);
        static void GetMouseDelta(float& dx, float& dy);
        static void SetMouseLocked(bool locked);
        static bool IsMouseLocked();

    private:
        static GLFWwindow* s_window;
        static std::unordered_map<int, bool> s_keyStates;
        static std::unordered_map<int, bool> s_keyStatesPrevious;
        static std::unordered_map<int, bool> s_mouseButtonStates;
        static std::unordered_map<int, bool> s_mouseButtonStatesPrevious;
        static float s_mouseX;
        static float s_mouseY;
        static float s_mouseDeltaX;
        static float s_mouseDeltaY;
        static bool s_mouseLocked;
    };
}

#endif