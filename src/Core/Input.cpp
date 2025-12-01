/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "Core/Input.h"

namespace MinecraftClone
{
    GLFWwindow* Input::s_window = nullptr;
    std::unordered_map<int, bool> Input::s_keyStates;
    std::unordered_map<int, bool> Input::s_keyStatesPrevious;
    std::unordered_map<int, bool> Input::s_mouseButtonStates;
    std::unordered_map<int, bool> Input::s_mouseButtonStatesPrevious;
    float Input::s_mouseX = 0.0f;
    float Input::s_mouseY = 0.0f;
    float Input::s_mouseDeltaX = 0.0f;
    float Input::s_mouseDeltaY = 0.0f;
    bool Input::s_mouseLocked = false;

    void Input::Initialize(GLFWwindow* window)
    {
        s_window = window;
    }

    void Input::Update()
    {
        // Save previous frame states
        s_keyStatesPrevious = s_keyStates;
        s_mouseButtonStatesPrevious = s_mouseButtonStates;

        // Update keyboard states
        for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; key++)
        {
            s_keyStates[key] = glfwGetKey(s_window, key) == GLFW_PRESS;
        }

        // Update mouse button states
        for (int button = GLFW_MOUSE_BUTTON_1; button <= GLFW_MOUSE_BUTTON_LAST; button++)
        {
            s_mouseButtonStates[button] = glfwGetMouseButton(s_window, button) == GLFW_PRESS;
        }

        // Update mouse position
        double x, y;
        glfwGetCursorPos(s_window, &x, &y);

        float newX = static_cast<float>(x);
        float newY = static_cast<float>(y);

        s_mouseDeltaX = newX - s_mouseX;
        s_mouseDeltaY = newY - s_mouseY;

        s_mouseX = newX;
        s_mouseY = newY;

        // Handle mouse locking
        if (s_mouseLocked)
        {
            int width, height;
            glfwGetWindowSize(s_window, &width, &height);
            glfwSetCursorPos(s_window, width / 2.0, height / 2.0);
            s_mouseX = width / 2.0f;
            s_mouseY = height / 2.0f;
        }
    }

    void Input::Shutdown()
    {
        s_window = nullptr;
        s_keyStates.clear();
        s_keyStatesPrevious.clear();
        s_mouseButtonStates.clear();
        s_mouseButtonStatesPrevious.clear();
    }

    bool Input::IsKeyPressed(int key)
    {
        return s_keyStates[key] && !s_keyStatesPrevious[key];
    }

    bool Input::IsKeyDown(int key)
    {
        return s_keyStates[key];
    }

    bool Input::IsKeyReleased(int key)
    {
        return !s_keyStates[key] && s_keyStatesPrevious[key];
    }

    bool Input::IsMouseButtonPressed(int button)
    {
        return s_mouseButtonStates[button] && !s_mouseButtonStatesPrevious[button];
    }

    bool Input::IsMouseButtonDown(int button)
    {
        return s_mouseButtonStates[button];
    }

    bool Input::IsMouseButtonReleased(int button)
    {
        return !s_mouseButtonStates[button] && s_mouseButtonStatesPrevious[button];
    }

    void Input::GetMousePosition(float& x, float& y)
    {
        x = s_mouseX;
        y = s_mouseY;
    }

    void Input::GetMouseDelta(float& dx, float& dy)
    {
        dx = s_mouseDeltaX;
        dy = s_mouseDeltaY;
    }

    void Input::SetMouseLocked(bool locked)
    {
        s_mouseLocked = locked;
        if (locked)
        {
            glfwSetInputMode(s_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
        {
            glfwSetInputMode(s_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    bool Input::IsMouseLocked()
    {
        return s_mouseLocked;
    }
}