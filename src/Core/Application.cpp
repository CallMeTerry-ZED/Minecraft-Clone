/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

// MUST Include spdlog FIRST before any yojimbo headers to avoid macro conflicts
#include <spdlog/spdlog.h>

#include "Core/Application.h"
#include "Core/Time.h"
#include "Core/Input.h"
#include "Core/Event.h"
#include "Core/EventDispatcher.h"
#include "Core/Camera.h"
#include "Rendering/TestCube.h"
#include "World/BlockType.h"
#include "World/World.h"
#include "World/ChunkRenderer.h"
#include "World/TerrainGenerator.h"
#include "World/ChunkManager.h"
#include "World/BlockInteraction.h"
#include "Networking/NetworkManager.h"
#include "Rendering/RemotePlayerRenderer.h"

// ImGui includes
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <yojimbo.h>
#include <chrono>
#include <thread>

namespace MinecraftClone
{
    Application::Application()
        : m_window(nullptr)
        , m_windowWidth(1280)
        , m_windowHeight(720)
        , m_running(false)
    {
        m_eventDispatcher = std::make_unique<EventDispatcher>();
    }

    Application::~Application()
    {
        Shutdown();
    }

    bool Application::Initialize()
    {
        spdlog::info("Initializing Minecraft Clone...");

        // Initialize GLFW
        if (!glfwInit())
        {
            spdlog::error("Failed to initialize GLFW!");
            return false;
        }

        // Configure GLFW for OpenGL 4.6
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        // Create window
        m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "Minecraft Clone", nullptr, nullptr);
        if (!m_window)
        {
            spdlog::error("Failed to create GLFW window!");
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(m_window);

        // Initialize Input system with the window
        Input::Initialize(m_window);

        // Load OpenGL functions with GLAD
        if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
        {
            spdlog::error("Failed to initialize GLAD!");
            glfwDestroyWindow(m_window);
            glfwTerminate();
            return false;
        }

        spdlog::info("Loaded OpenGL {}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));

        // Set window user pointer for callbacks
        glfwSetWindowUserPointer(m_window, this);

        // Set GLFW callbacks
        glfwSetWindowCloseCallback(m_window, WindowCloseCallback);
        glfwSetWindowSizeCallback(m_window, WindowResizeCallback);
        glfwSetKeyCallback(m_window, KeyCallback);
        glfwSetMouseButtonCallback(m_window, MouseButtonCallback);
        glfwSetCursorPosCallback(m_window, CursorPosCallback);
        glfwSetScrollCallback(m_window, ScrollCallback);

        // Get window position and center it
        int monitorX, monitorY, monitorWidth, monitorHeight;
        glfwGetMonitorPos(glfwGetPrimaryMonitor(), &monitorX, &monitorY);
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        monitorWidth = mode->width;
        monitorHeight = mode->height;

        int windowX = monitorX + (monitorWidth - m_windowWidth) / 2;
        int windowY = monitorY + (monitorHeight - m_windowHeight) / 2;
        glfwSetWindowPos(m_window, windowX, windowY);

        spdlog::info("Window position: {}, {}", windowX, windowY);
        spdlog::info("Window centered at: {}, {}", (monitorWidth - m_windowWidth) / 2, (monitorHeight - m_windowHeight) / 2);

        // Make window visible
        glfwShowWindow(m_window);
        glfwFocusWindow(m_window);
        glfwRequestWindowAttention(m_window);
        glfwRestoreWindow(m_window);
        glfwPollEvents();

        spdlog::info("Window is now visible");

        glfwPollEvents();

        // Basic OpenGL setup
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);  // Add this for proper depth testing
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f); // Sky blue

        // Initialize ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking

        // Setup ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        const char* glsl_version = "#version 460";
        ImGui_ImplOpenGL3_Init(glsl_version);

        m_imguiInitialized = true;
        spdlog::info("ImGui initialized");

        // Initialize block registry
        BlockRegistry::Initialize();

        // Initialize world
        m_world = std::make_unique<World>();

        // Initialize terrain generator
        m_terrainGenerator = std::make_unique<TerrainGenerator>();
        m_terrainGenerator->Initialize(12345); // Seed

        // Initialize chunk renderer
        m_chunkRenderer = std::make_unique<ChunkRenderer>();
        if (!m_chunkRenderer->Initialize())
        {
            spdlog::error("Failed to initialize chunk renderer!");
            return false;
        }

        m_remotePlayerRenderer = std::make_unique<RemotePlayerRenderer>();
        if (!m_remotePlayerRenderer->Initialize())
        {
            spdlog::error("Failed to initialize remote player renderer!");
            return false;
        }

        // Initialize camera
        m_camera = std::make_unique<Camera>();
        m_camera->SetAspectRatio(static_cast<float>(m_windowWidth) / static_cast<float>(m_windowHeight));
        m_camera->SetPosition(glm::vec3(0.0f, 100.0f, 0.0f)); // Start high above the world

        // Initialize chunk manager
        m_chunkManager = std::make_unique<ChunkManager>();
        m_chunkManager->Initialize(m_world.get(), m_terrainGenerator.get(), m_chunkRenderer.get());
        m_chunkManager->SetRenderDistance(8);  // 8 chunks render distance

        // Initialize block interaction
        m_blockInteraction = std::make_unique<BlockInteraction>();
        m_blockInteraction->Initialize(m_world.get(), m_chunkRenderer.get(), m_chunkManager.get());
        m_blockInteraction->SetSelectedBlockType(BlockType::Stone); // Default block to place

        // Initialize yojimbo
        if (!InitializeYojimbo())
        {
            spdlog::error("Failed to initialize Yojimbo!");
            return false;
        }

        // Initialize network manager
        m_networkManager = std::make_unique<NetworkManager>();

        // Generate initial terrain around spawn
        // Only generate on server - clients will receive chunks from server
        if (!m_networkManager || m_networkManager->IsServerRunning())
        {
            GenerateTerrainWorld();
        }
        else
        {
            spdlog::info("Client mode: Will receive terrain from server");
        }

        // Wire world / renderer into network manager so it can apply block updates
        m_networkManager->SetWorld(m_world.get());
        m_networkManager->SetChunkRenderer(m_chunkRenderer.get());

        // Set network manager in block interaction (so local edits can send updates)
        m_blockInteraction->SetNetworkManager(m_networkManager.get());

        // Networking is ready but not started by default
        // Press F1 to start server, F2 to connect as client
        spdlog::info("Networking ready! Press F1 to start server, F2 to connect as client");

        // Set running flag to true
        m_running = true;

        spdlog::info("Application initialized successfully!");
        return true;
    }

    void Application::Run()
    {
        spdlog::info("Entering main loop...");

        // Ensure window is visible before entering loop
        if (glfwGetWindowAttrib(m_window, GLFW_VISIBLE) == GLFW_FALSE)
        {
            spdlog::warn("Window is not visible! Attempting to show...");
            glfwShowWindow(m_window);
            glfwPollEvents();
        }

        if (glfwGetWindowAttrib(m_window, GLFW_ICONIFIED) == GLFW_TRUE)
        {
            spdlog::warn("Window is minimized! Attempting to restore...");
            glfwRestoreWindow(m_window);
            glfwPollEvents();
        }

        // Force window to front
        glfwFocusWindow(m_window);
        glfwRequestWindowAttention(m_window);
        glfwPollEvents();

        auto lastTime = std::chrono::high_resolution_clock::now();
        int frameCount = 0;

        while (m_running && !glfwWindowShouldClose(m_window))
        {
            // Calculate delta time
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - lastTime);
            float deltaTime = duration.count() / 1000000.0f;
            lastTime = currentTime;

            if (deltaTime > 0.1f) deltaTime = 0.1f;
            if (deltaTime <= 0.0f) deltaTime = 0.0001f;

            Time::Update(deltaTime);
            ProcessEvents();
            DispatchEvents();  // Process queued events
            Input::Update();   // Update input state
            Update(deltaTime);
            Render();

            // Swap buffers
            glfwSwapBuffers(m_window);

            // Small sleep to prevent 100% CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(1));

            frameCount++;
        }

        spdlog::info("Exiting main loop...");
    }

    void Application::ProcessEvents()
    {
        glfwPollEvents();
    }

    void Application::Update(float deltaTime)
    {
        (void)deltaTime; // Suppress unused parameter warning

        // Update network manager
        if (m_networkManager)
        {
            static double networkTime = 0.0;
            networkTime += deltaTime;
            m_networkManager->Update(networkTime, deltaTime);

            // Send player position updates (every frame when connected)
            if (m_camera && (m_networkManager->IsConnected() || m_networkManager->IsServerRunning()))
            {
                m_networkManager->SendPlayerPosition(
                    m_camera->GetPosition(),
                    m_camera->GetYaw(),
                    m_camera->GetPitch()
                );
            }
        }

        // Update camera
        if (m_camera)
        {
            m_camera->Update(deltaTime);

            // Update chunk manager with player position
            if (m_chunkManager)
            {
                m_chunkManager->Update(m_camera->GetPosition());
            }

            // Update block interaction (raycast)
            if (m_blockInteraction)
            {
                m_blockInteraction->Update(m_camera.get(), 5.0f); // 5 block reach distance
            }
        }
    }

    void Application::Render()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (m_camera)
        {
            if (m_chunkRenderer)
            {
                m_chunkRenderer->RenderChunks(m_camera->GetViewMatrix(), m_camera->GetProjectionMatrix());
            }

            // Keep test cube for now (can remove later)
            if (m_testCube)
            {
                m_testCube->Render(m_camera->GetViewMatrix(), m_camera->GetProjectionMatrix());
            }

            if (m_networkManager && m_remotePlayerRenderer)
            {
                const auto& players = m_networkManager->GetRemotePlayers();
                m_remotePlayerRenderer->Render(players,
                                               m_camera->GetViewMatrix(),
                                               m_camera->GetProjectionMatrix());
            }
        }

        // Render ImGui
        if (m_imguiInitialized)
        {
            RenderImGui();
        }
    }

   void Application::RenderImGui()
    {
        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Debug Window
        {
            ImGui::Begin("Debug Info");

            // FPS
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);

            ImGui::Separator();

            // Camera Info
            if (m_camera)
            {
                glm::vec3 pos = m_camera->GetPosition();
                ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
                ImGui::Text("Camera Yaw: %.2f", m_camera->GetYaw());
                ImGui::Text("Camera Pitch: %.2f", m_camera->GetPitch());
            }

            ImGui::Separator();

            // World Info
            if (m_world)
            {
                ImGui::Text("World: Loaded");
            }

            if (m_chunkManager)
            {
                ImGui::Text("Chunk Manager: Active");
            }

            ImGui::Separator();

            // Networking Info
            if (m_networkManager)
            {
                if (m_networkManager->IsServerRunning())
                {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Server: RUNNING");
                    ImGui::Text("Port: 40000");
                }
                else if (m_networkManager->IsConnected())
                {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Client: CONNECTED");
                }
                else if (m_networkManager->IsConnecting())
                {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Client: CONNECTING...");
                }
                else
                {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Network: DISCONNECTED");
                }

                ImGui::Text("Player ID: %u", m_networkManager->GetLocalPlayerId());
            }

            ImGui::Separator();

            // Controls
            ImGui::Text("Controls:");
            ImGui::BulletText("Tab - Toggle Mouse Lock");
            ImGui::BulletText("F1 - Start Server");
            ImGui::BulletText("F2 - Connect as Client");
            ImGui::BulletText("F3 - Disconnect/Stop");
            ImGui::BulletText("Left Click - Break Block");
            ImGui::BulletText("Right Click - Place Block");

            ImGui::End();
        }

        // Networking Window
        {
            ImGui::Begin("Networking");

            if (m_networkManager)
            {
                if (ImGui::Button("Start Server (F1)"))
                {
                    if (!m_networkManager->IsServerRunning())
                    {
                        if (m_networkManager->StartServer("127.0.0.1", 40000))
                        {
                            spdlog::info("Server started via ImGui");
                        }
                    }
                }

                ImGui::SameLine();

                if (ImGui::Button("Connect Client (F2)"))
                {
                    if (!m_networkManager->IsConnected() && !m_networkManager->IsServerRunning())
                    {
                        if (m_networkManager->ConnectToServer("127.0.0.1", 40000))
                        {
                            spdlog::info("Client connecting via ImGui");
                        }
                    }
                }

                ImGui::SameLine();

                if (ImGui::Button("Disconnect (F3)"))
                {
                    if (m_networkManager->IsServerRunning())
                    {
                        m_networkManager->StopServer();
                    }
                    else if (m_networkManager->IsConnected())
                    {
                        m_networkManager->Disconnect();
                    }
                }

                ImGui::Separator();

                // Network status
                bool isServer = m_networkManager->IsServerRunning();
                bool isClient = m_networkManager->IsConnected();

                if (isServer)
                {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Status: Server Running");
                }
                else if (isClient)
                {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Status: Client Connected");
                }
                else
                {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Status: Disconnected");
                }
            }

            ImGui::End();
        }

        // Render ImGui
        ImGui::Render();

        // Save OpenGL state before ImGui rendering
        GLint last_program, last_texture, last_array_buffer, last_element_array_buffer, last_vertex_array;
        GLint last_viewport[4], last_scissor_box[4];
        GLenum last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha;
        GLenum last_blend_equation_rgb, last_blend_equation_alpha;
        GLboolean last_enable_blend, last_enable_cull_face, last_enable_depth_test, last_enable_scissor_test;

        glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
        glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
        glGetIntegerv(GL_VIEWPORT, last_viewport);
        glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
        glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
        glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
        glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
        glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
        glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
        last_enable_blend = glIsEnabled(GL_BLEND);
        last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
        last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
        last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

        // Render ImGui
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Restore OpenGL state after ImGui rendering
        glUseProgram(last_program);
        glBindTexture(GL_TEXTURE_2D, last_texture);
        glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
        glBindVertexArray(last_vertex_array);
        glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
        glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
        if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
        if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
        if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
        glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
        glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
    }

    void Application::Shutdown()
    {
        if (m_networkManager)
        {
            m_networkManager->StopServer();
            m_networkManager->Disconnect();
            m_networkManager.reset();
        }

        if (m_chunkManager)
        {
            m_chunkManager->Shutdown();
            m_chunkManager.reset();
        }

        if (m_remotePlayerRenderer)
        {
            m_remotePlayerRenderer->Shutdown();
            m_remotePlayerRenderer.reset();
        }

        if (m_chunkRenderer)
        {
            m_chunkRenderer->Shutdown();
            m_chunkRenderer.reset();
        }

        if (m_testCube)
        {
            m_testCube->Shutdown();
            m_testCube.reset();
        }

        Input::Shutdown();

        if (m_imguiInitialized)
        {
            ImGui_ImplOpenGL3_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
            m_imguiInitialized = false;
        }

        if (m_window)
        {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }

        glfwTerminate();
        ShutdownYojimbo();
        spdlog::info("Application shut down");
    }

    void Application::OnEvent(Event& event)
    {
        // Default event handling - can be overridden
        if (event.GetEventType() == EventType::WindowClose)
        {
            m_running = false;
        }
        else if (event.GetEventType() == EventType::WindowResize)
        {
            WindowResizeEvent& resizeEvent = static_cast<WindowResizeEvent&>(event);
            m_windowWidth = resizeEvent.GetWidth();
            m_windowHeight = resizeEvent.GetHeight();
            glViewport(0, 0, m_windowWidth, m_windowHeight);

            if (m_camera)
            {
                m_camera->SetAspectRatio(static_cast<float>(m_windowWidth) / static_cast<float>(m_windowHeight));
            }
        }
        else if (event.GetEventType() == EventType::KeyPressed)
        {
            // Check if ImGui wants keyboard input
            if (m_imguiInitialized && ImGui::GetIO().WantCaptureKeyboard)
            {
                // ImGui is using keyboard, skip game input
                return;
            }

            KeyPressedEvent& keyEvent = static_cast<KeyPressedEvent&>(event);

            // Toggle mouse lock with Tab key
            if (keyEvent.GetKey() == GLFW_KEY_TAB)
            {
                Input::SetMouseLocked(!Input::IsMouseLocked());
            }
            // F1 - Start server
            else if (keyEvent.GetKey() == GLFW_KEY_F1)
            {
                if (m_networkManager && !m_networkManager->IsServerRunning())
                {
                    if (m_networkManager->StartServer("127.0.0.1", 40000))
                    {
                        spdlog::info("Server started on 127.0.0.1:40000");
                    }
                    else
                    {
                        spdlog::error("Failed to start server!");
                    }
                }
                else
                {
                    spdlog::warn("Server is already running!");
                }
            }
            // F2 - Connect as client
            else if (keyEvent.GetKey() == GLFW_KEY_F2)
            {
                if (m_networkManager && !m_networkManager->IsConnected())
                {
                    if (m_networkManager->ConnectToServer("127.0.0.1", 40000))
                    {
                        spdlog::info("Connecting to server 127.0.0.1:40000...");
                    }
                    else
                    {
                        spdlog::error("Failed to connect to server!");
                    }
                }
                else
                {
                    spdlog::warn("Already connected or server is running!");
                }
            }
            // F3 - Disconnect/Stop server
            else if (keyEvent.GetKey() == GLFW_KEY_F3)
            {
                if (m_networkManager)
                {
                    if (m_networkManager->IsServerRunning())
                    {
                        m_networkManager->StopServer();
                        spdlog::info("Server stopped");
                    }
                    else if (m_networkManager->IsConnected())
                    {
                        m_networkManager->Disconnect();
                        spdlog::info("Disconnected from server");
                    }
                }
            }
        }
        else if (event.GetEventType() == EventType::MouseButtonPressed)
        {
            // Check if ImGui wants mouse input
            if (m_imguiInitialized && ImGui::GetIO().WantCaptureMouse)
            {
                // ImGui is using mouse, skip game input
                return;
            }

            MouseButtonPressedEvent& mouseEvent = static_cast<MouseButtonPressedEvent&>(event);

            if (m_blockInteraction)
            {
                // Left click = break block
                if (mouseEvent.GetButton() == GLFW_MOUSE_BUTTON_LEFT)
                {
                    m_blockInteraction->BreakBlock();
                }
                // Right click = place block
                else if (mouseEvent.GetButton() == GLFW_MOUSE_BUTTON_RIGHT)
                {
                    m_blockInteraction->PlaceBlock(m_blockInteraction->GetSelectedBlockType());
                }
            }
        }
    }

    void Application::QueueEvent(std::unique_ptr<Event> event)
    {
        m_eventQueue.push(std::move(event));
    }

    void Application::DispatchEvents()
    {
        while (!m_eventQueue.empty())
        {
            auto event = std::move(m_eventQueue.front());
            m_eventQueue.pop();

            // Dispatch to application first
            OnEvent(*event);

            // Then dispatch to subscribers
            if (!event->m_handled)
            {
                m_eventDispatcher->Dispatch(*event);
            }
        }
    }

    // GLFW Callbacks
    void Application::WindowCloseCallback(GLFWwindow* window)
    {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        app->QueueEvent(std::make_unique<WindowCloseEvent>());
    }

    void Application::WindowResizeCallback(GLFWwindow* window, int width, int height)
    {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        app->QueueEvent(std::make_unique<WindowResizeEvent>(width, height));
    }

    void Application::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));

        switch (action)
        {
            case GLFW_PRESS:
                app->QueueEvent(std::make_unique<KeyPressedEvent>(key, scancode, mods, false));
                break;
            case GLFW_RELEASE:
                app->QueueEvent(std::make_unique<KeyReleasedEvent>(key, scancode, mods));
                break;
            case GLFW_REPEAT:
                app->QueueEvent(std::make_unique<KeyPressedEvent>(key, scancode, mods, true));
                break;
        }
    }

    void Application::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));

        if (action == GLFW_PRESS)
        {
            app->QueueEvent(std::make_unique<MouseButtonPressedEvent>(button, mods));
        }
        else if (action == GLFW_RELEASE)
        {
            app->QueueEvent(std::make_unique<MouseButtonReleasedEvent>(button, mods));
        }
    }

    void Application::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        app->QueueEvent(std::make_unique<MouseMovedEvent>(static_cast<float>(xpos), static_cast<float>(ypos)));
    }

    void Application::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
    {
        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        app->QueueEvent(std::make_unique<MouseScrolledEvent>(static_cast<float>(xoffset), static_cast<float>(yoffset)));
    }

    void Application::GenerateTestWorld()
    {
        spdlog::info("Generating test world...");

        // Create a simple flat world with some blocks
        for (int x = 0; x < 32; x++)
        {
            for (int z = 0; z < 32; z++)
            {
                // Create a flat ground at y=10
                m_world->SetBlock(x, 10, z, BlockType::Grass);
                m_world->SetBlock(x, 9, z, BlockType::Dirt);
                m_world->SetBlock(x, 8, z, BlockType::Dirt);
                m_world->SetBlock(x, 7, z, BlockType::Stone);
            }
        }

        // Add some test blocks
        m_world->SetBlock(15, 11, 15, BlockType::Wood);
        m_world->SetBlock(15, 12, 15, BlockType::Wood);
        m_world->SetBlock(15, 13, 15, BlockType::Leaves);
        m_world->SetBlock(15, 14, 15, BlockType::Leaves);

        m_world->SetBlock(20, 11, 20, BlockType::Stone);
        m_world->SetBlock(20, 12, 20, BlockType::Cobblestone);

        // Update chunk meshes
        for (int chunkX = 0; chunkX < 2; chunkX++)
        {
            for (int chunkZ = 0; chunkZ < 2; chunkZ++)
            {
                Chunk* chunk = m_world->GetChunk(chunkX, chunkZ);
                if (chunk)
                {
                    m_chunkRenderer->UpdateChunk(chunk, chunkX, chunkZ, m_world.get());
                }
            }
        }

        spdlog::info("Test world generated!");
    }

    void Application::GenerateTerrainWorld()
    {
        // Only generate terrain on server
        if (m_networkManager && !m_networkManager->IsServerRunning())
        {
            spdlog::info("Client: Waiting for chunk data from server instead of generating terrain");
            return;
        }

        spdlog::info("Generating initial terrain...");

        // Generate initial chunks around spawn (0, 0)
        // ChunkManager will handle dynamic loading from here
        const int initialRadius = 3;
        int centerChunkX = 0;
        int centerChunkZ = 0;

        for (int chunkX = -initialRadius; chunkX <= initialRadius; chunkX++)
        {
            for (int chunkZ = -initialRadius; chunkZ <= initialRadius; chunkZ++)
            {
                int worldChunkX = centerChunkX + chunkX;
                int worldChunkZ = centerChunkZ + chunkZ;

                Chunk* chunk = m_world->GetOrCreateChunk(worldChunkX, worldChunkZ);

                // Generate terrain for this chunk
                m_terrainGenerator->GenerateChunk(chunk, worldChunkX, worldChunkZ, m_world.get());

                // Update mesh
                m_chunkRenderer->UpdateChunk(chunk, worldChunkX, worldChunkZ, m_world.get());
            }
        }

        spdlog::info("Initial terrain generated! {} chunks", (initialRadius * 2 + 1) * (initialRadius * 2 + 1));
    }
}