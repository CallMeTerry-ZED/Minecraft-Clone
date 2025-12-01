cmake_minimum_required(VERSION 3.20)
project(MinecraftClone VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ============================================================================
# Source files (using GLOB_RECURSE)
# ============================================================================

file(GLOB_RECURSE SOURCE_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx"
)

file(GLOB_RECURSE HEADER_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hxx"
)

# ============================================================================
# Third-party libraries
# ============================================================================

# GLM - Math library (header-only)
add_subdirectory(thirdparty/glm)

# nlohmann/json - JSON library (header-only)
add_subdirectory(thirdparty/json)

# FastNoise2 - Terrain generation
add_subdirectory(thirdparty/fastnoise2)

# zlib - Compression
add_subdirectory(thirdparty/zlib)

# entt - Entity Component System (header-only)
add_subdirectory(thirdparty/entt)

# spdlog - Logging library
add_subdirectory(thirdparty/spdlog)

# ImGui - Immediate mode GUI
add_subdirectory(thirdparty/imgui)

# GLFW - Window management
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build GLFW documentation" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build GLFW tests" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build GLFW examples" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "Generate GLFW installation target" FORCE)
add_subdirectory(thirdparty/glfw)

# Bullet3 - Physics engine
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)
set(USE_GRAPHICAL_BENCHMARK OFF CACHE BOOL "Use Graphical Benchmark" FORCE)
set(BUILD_BULLET2_DEMOS OFF CACHE BOOL "Build Bullet2 demos" FORCE)
set(BUILD_BULLET3 OFF CACHE BOOL "Build Bullet3" FORCE)
set(BUILD_EXTRAS OFF CACHE BOOL "Build extras" FORCE)
set(BUILD_UNIT_TESTS OFF CACHE BOOL "Build unit tests" FORCE)
set(INSTALL_LIBS OFF CACHE BOOL "Install libraries" FORCE)
add_subdirectory(thirdparty/bullet3 EXCLUDE_FROM_ALL)

# GLAD - OpenGL loader
# Note: GLAD files need to be generated first
# You can generate them using: python -m glad --api gl:core=4.6 --out-path generated c
# Generated files exist in thirdparty/glad/generated
set(GLAD_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glad/generated/include)
set(GLAD_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/glad/generated/src)

# Check if GLAD files exist, if not, provide instructions
if(NOT EXISTS ${GLAD_INCLUDE_DIR}/glad/gl.h)
    message(WARNING "GLAD files not found! Please generate them using:")
    message(WARNING "  cd thirdparty/glad")
    message(WARNING "  python -m glad --api gl:core=4.6 --out-path generated c")
    message(WARNING "Or use the web service: https://glad.sh")
else()
    # Add GLAD source file to the build
    list(APPEND SOURCE_FILES ${GLAD_SOURCE_DIR}/gl.c)
    message(STATUS "GLAD found: ${GLAD_INCLUDE_DIR}")
endif()

# SoLoud - Audio engine
set(SOLOUD_STATIC ON CACHE BOOL "Build static SoLoud" FORCE)
set(SOLOUD_DYNAMIC OFF CACHE BOOL "Build dynamic SoLoud" FORCE)
set(SOLOUD_C_API OFF CACHE BOOL "Build C API" FORCE)
set(SOLOUD_BUILD_DEMOS OFF CACHE BOOL "Build demos" FORCE)

# Backend selection based on platform
if(UNIX AND NOT APPLE)
    set(SOLOUD_BACKEND_ALSA ON CACHE BOOL "ALSA backend" FORCE)
    set(SOLOUD_BACKEND_NULL OFF CACHE BOOL "NULL backend" FORCE)
    set(SOLOUD_BACKEND_SDL2 OFF CACHE BOOL "SDL2 backend" FORCE)
elseif(APPLE)
    set(SOLOUD_BACKEND_COREAUDIO ON CACHE BOOL "CoreAudio backend" FORCE)
    set(SOLOUD_BACKEND_NULL OFF CACHE BOOL "NULL backend" FORCE)
    set(SOLOUD_BACKEND_SDL2 OFF CACHE BOOL "SDL2 backend" FORCE)
elseif(WIN32)
    set(SOLOUD_BACKEND_WASAPI ON CACHE BOOL "WASAPI backend" FORCE)
    set(SOLOUD_BACKEND_NULL OFF CACHE BOOL "NULL backend" FORCE)
    set(SOLOUD_BACKEND_SDL2 OFF CACHE BOOL "SDL2 backend" FORCE)
endif()

set(SOLOUD_BACKEND_XAUDIO2 OFF CACHE BOOL "XAudio2 backend" FORCE)
set(SOLOUD_BACKEND_WINMM OFF CACHE BOOL "WINMM backend" FORCE)
set(SOLOUD_BACKEND_OPENSLES OFF CACHE BOOL "OpenSLES backend" FORCE)
set(SOLOUD_GENERATE_GLUE OFF CACHE BOOL "Generate Glue" FORCE)

add_subdirectory(thirdparty/soloud)

# yojimbo - Network library
if(UNIX AND NOT APPLE)
    set(YOJIMBO_USE_SYSTEM_SODIUM ON CACHE BOOL "Use system libsodium" FORCE)
endif()
set(YOJIMBO_BUILD_TESTS OFF CACHE BOOL "Build yojimbo tests" FORCE)
add_subdirectory(thirdparty/yojimbo)

# OpenGL
find_package(OpenGL REQUIRED)

# ============================================================================
# Executable
# ============================================================================

add_executable(${PROJECT_NAME}
        ${SOURCE_FILES}
        ${HEADER_FILES}
        include/Core/Event.h
        include/Core/EventDispatcher.h
        src/Core/Input.cpp
        include/Core/Input.h
        src/Core/Camera.cpp
        include/Core/Camera.h
        src/Rendering/Shader.cpp
        include/Rendering/Shader.h
        src/Rendering/TestCube.cpp
        include/Rendering/TestCube.h
        src/World/BlockType.cpp
        include/World/BlockType.h
        src/World/Block.cpp
        include/World/Block.h
        src/World/Chunk.cpp
        include/World/Chunk.h
        src/World/World.cpp
        include/World/World.h
        src/Rendering/ChunkMesh.cpp
        include/Rendering/ChunkMesh.h
        src/World/ChunkMeshGenerator.cpp
        include/World/ChunkMeshGenerator.h
        src/World/ChunkRenderer.cpp
        include/World/ChunkRenderer.h
        src/World/TerrainGenerator.cpp
        include/World/TerrainGenerator.h
        src/World/ChunkManager.cpp
        include/World/ChunkManager.h
        src/World/Raycast.cpp
        include/World/Raycast.h
        src/World/BlockInteraction.cpp
        include/World/BlockInteraction.h
        include/Networking/NetworkMessages.h
        src/Networking/NetworkManager.cpp
        include/Networking/NetworkManager.h
        src/Networking/NetworkMessages.cpp
        include/Networking/YojimboMessageFactory.h
        src/Rendering/RemotePlayerRenderer.cpp
        include/Rendering/RemotePlayerRenderer.h
        src/Rendering/Texture.cpp
        include/Rendering/Texture.h
        src/Rendering/BlockTextureRegistry.cpp
        include/Rendering/BlockTextureRegistry.h
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/stb        # For stb_image.h, etc.
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/soloud/include  # SoLoud headers
        ${GLAD_INCLUDE_DIR}  # GLAD headers (if generated)
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/bullet3/src  # Bullet3 headers
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
        # Math
        glm::glm

        # JSON
        nlohmann_json::nlohmann_json

        # Noise generation
        FastNoise2

        # Compression
        zlibstatic

        # ECS
        EnTT::EnTT

        # Logging
        spdlog::spdlog

        # Audio
        soloud

        # Networking
        yojimbo

        # Physics
        BulletDynamics
        BulletCollision
        LinearMath

        # Windowing
        glfw

        # Graphics
        OpenGL::GL

        # GUI
        imgui
)

# X11 libraries (required for ImGui GLFW backend on Linux)
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
            X11
            Xrandr
    )
endif()

# Compiler-specific options
if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
            _CRT_SECURE_NO_WARNINGS
    )
    target_compile_options(${PROJECT_NAME} PRIVATE
            /W4
            /permissive-
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
            -Wall
            -Wextra
            -Wpedantic
    )
endif()

# Debug/Release specific options
target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
        $<$<CONFIG:Debug>:_DEBUG>
)

# ============================================================================
# Copy assets folder to output directory
# ============================================================================

# Copy assets folder next to the executable
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets"
    COMMENT "Copying assets folder to output directory"
)

# ============================================================================
# Installation (optional)
# ============================================================================

install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)

# ============================================================================
# Print configuration summary
# ============================================================================

message(STATUS "")
message(STATUS "=== Minecraft Clone Configuration ===")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "OpenGL found: ${OpenGL_FOUND}")
message(STATUS "====================================")
message(STATUS "")