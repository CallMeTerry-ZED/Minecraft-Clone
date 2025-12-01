cmake_minimum_required(VERSION 3.20)

# ImGui library
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# ImGui source files
set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# ImGui header files
set(IMGUI_HEADERS
        ${IMGUI_DIR}/imgui.h
        ${IMGUI_DIR}/imgui_internal.h
        ${IMGUI_DIR}/imconfig.h
        ${IMGUI_DIR}/imstb_rectpack.h
        ${IMGUI_DIR}/imstb_textedit.h
        ${IMGUI_DIR}/imstb_truetype.h
        ${IMGUI_DIR}/backends/imgui_impl_glfw.h
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.h
        ${IMGUI_DIR}/backends/imgui_impl_opengl3_loader.h
)

# Create ImGui library
add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_HEADERS})

# Include directories
target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
)

# Link GLFW (required for imgui_impl_glfw)
target_link_libraries(imgui PUBLIC glfw)

# Link OpenGL (required for imgui_impl_opengl3)
find_package(OpenGL REQUIRED)
target_link_libraries(imgui PUBLIC OpenGL::GL)

# Set C++ standard
target_compile_features(imgui PUBLIC cxx_std_17)

# Disable warnings for ImGui
if(MSVC)
    target_compile_options(imgui PRIVATE /W4)
else()
    target_compile_options(imgui PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Export ImGui target
set(IMGUI_FOUND TRUE CACHE BOOL "ImGui found" FORCE)