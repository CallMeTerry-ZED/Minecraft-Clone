/*
* © 2025 ZED Interactive. All Rights Reserved.
 */

#ifndef APPLICATION_H
#define APPLICATION_H

#pragma once

// Include GLAD BEFORE GLFW to prevent GLFW from including system OpenGL headers
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <spdlog/spdlog.h>
#include <queue>

namespace MinecraftClone
{
    class Event;
    class EventDispatcher;
    class RemotePlayerRenderer;

    class Application
    {
    public:
        Application();
        ~Application();

        bool Initialize();
        void Run();
        void Shutdown();

        void OnEvent(Event& event);

        GLFWwindow* GetWindow() const { return m_window; }
        int GetWidth() const { return m_windowWidth; }
        int GetHeight() const { return m_windowHeight; }

    private:
        void ProcessEvents();
        void Update(float deltaTime);
        void Render();
        void RenderImGui();

        // Event handling
        void QueueEvent(std::unique_ptr<Event> event);
        void DispatchEvents();
        void GenerateTestWorld();
        void GenerateTerrainWorld();

        // GLFW callbacks
        static void WindowCloseCallback(GLFWwindow* window);
        static void WindowResizeCallback(GLFWwindow* window, int width, int height);
        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
        static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

        // GLFW
        GLFWwindow* m_window = nullptr;
        int m_windowWidth = 1280;
        int m_windowHeight = 720;

        // Event system
        std::queue<std::unique_ptr<Event>> m_eventQueue;
        std::unique_ptr<EventDispatcher> m_eventDispatcher;

        // Camera
        std::unique_ptr<class Camera> m_camera;

        // Rendering
        std::unique_ptr<class TestCube> m_testCube;
        std::unique_ptr<class ChunkRenderer> m_chunkRenderer;
        std::unique_ptr<RemotePlayerRenderer> m_remotePlayerRenderer;

        // World
        std::unique_ptr<class World> m_world;
        std::unique_ptr<class TerrainGenerator> m_terrainGenerator;
        std::unique_ptr<class ChunkManager> m_chunkManager;
        std::unique_ptr<class BlockInteraction> m_blockInteraction;
        std::unique_ptr<class NetworkManager> m_networkManager;

        // Physics
        std::unique_ptr<class PhysicsManager> m_physicsManager;

        // ImGui
        bool m_imguiInitialized = false;

        bool m_running = true;
    };
}

#endif