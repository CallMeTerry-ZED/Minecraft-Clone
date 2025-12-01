cmake_minimum_required(VERSION 3.20)

project(SoLoud VERSION 20200207 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Options
option(SOLOUD_STATIC "Build static library" ON)
option(SOLOUD_DYNAMIC "Build dynamic library" OFF)
option(SOLOUD_C_API "Build C API" OFF)
option(SOLOUD_BUILD_DEMOS "Build demos" OFF)

# Backend options
option(SOLOUD_BACKEND_NULL "NULL backend" ON)
option(SOLOUD_BACKEND_SDL2 "SDL2 backend" OFF)
option(SOLOUD_BACKEND_ALSA "ALSA backend" OFF)
option(SOLOUD_BACKEND_COREAUDIO "CoreAudio backend" OFF)
option(SOLOUD_BACKEND_OPENSLES "OpenSLES backend" OFF)
option(SOLOUD_BACKEND_XAUDIO2 "XAudio2 backend" OFF)
option(SOLOUD_BACKEND_WINMM "WINMM backend" OFF)
option(SOLOUD_BACKEND_WASAPI "WASAPI backend" OFF)

# Auto-detect backend based on platform
if(UNIX AND NOT APPLE)
    if(NOT SOLOUD_BACKEND_ALSA AND NOT SOLOUD_BACKEND_SDL2 AND NOT SOLOUD_BACKEND_NULL)
        set(SOLOUD_BACKEND_ALSA ON)
        message(STATUS "SoLoud: Auto-selected ALSA backend for Linux")
    endif()
elseif(APPLE)
    if(NOT SOLOUD_BACKEND_COREAUDIO AND NOT SOLOUD_BACKEND_SDL2 AND NOT SOLOUD_BACKEND_NULL)
        set(SOLOUD_BACKEND_COREAUDIO ON)
        message(STATUS "SoLoud: Auto-selected CoreAudio backend for macOS")
    endif()
elseif(WIN32)
    if(NOT SOLOUD_BACKEND_WASAPI AND NOT SOLOUD_BACKEND_XAUDIO2 AND NOT SOLOUD_BACKEND_WINMM AND NOT SOLOUD_BACKEND_SDL2 AND NOT SOLOUD_BACKEND_NULL)
        set(SOLOUD_BACKEND_WASAPI ON)
        message(STATUS "SoLoud: Auto-selected WASAPI backend for Windows")
    endif()
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Source files
set(SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Core sources
set(CORE_SOURCES
    ${SOURCES_DIR}/core/soloud.cpp
    ${SOURCES_DIR}/core/soloud_audiosource.cpp
    ${SOURCES_DIR}/core/soloud_bus.cpp
    ${SOURCES_DIR}/core/soloud_core_3d.cpp
    ${SOURCES_DIR}/core/soloud_core_basicops.cpp
    ${SOURCES_DIR}/core/soloud_core_faderops.cpp
    ${SOURCES_DIR}/core/soloud_core_filterops.cpp
    ${SOURCES_DIR}/core/soloud_core_getters.cpp
    ${SOURCES_DIR}/core/soloud_core_setters.cpp
    ${SOURCES_DIR}/core/soloud_core_voicegroup.cpp
    ${SOURCES_DIR}/core/soloud_core_voiceops.cpp
    ${SOURCES_DIR}/core/soloud_fader.cpp
    ${SOURCES_DIR}/core/soloud_fft.cpp
    ${SOURCES_DIR}/core/soloud_fft_lut.cpp
    ${SOURCES_DIR}/core/soloud_file.cpp
    ${SOURCES_DIR}/core/soloud_filter.cpp
    ${SOURCES_DIR}/core/soloud_misc.cpp
    ${SOURCES_DIR}/core/soloud_queue.cpp
    ${SOURCES_DIR}/core/soloud_thread.cpp
)

# Audio source files
set(AUDIOSOURCE_SOURCES
    ${SOURCES_DIR}/audiosource/ay/chipplayer.cpp
    ${SOURCES_DIR}/audiosource/ay/sndbuffer.cpp
    ${SOURCES_DIR}/audiosource/ay/sndchip.cpp
    ${SOURCES_DIR}/audiosource/ay/sndrender.cpp
    ${SOURCES_DIR}/audiosource/ay/soloud_ay.cpp
    ${SOURCES_DIR}/audiosource/monotone/soloud_monotone.cpp
    ${SOURCES_DIR}/audiosource/noise/soloud_noise.cpp
    ${SOURCES_DIR}/audiosource/openmpt/soloud_openmpt.cpp
    ${SOURCES_DIR}/audiosource/openmpt/soloud_openmpt_dll.c
    ${SOURCES_DIR}/audiosource/sfxr/soloud_sfxr.cpp
    ${SOURCES_DIR}/audiosource/speech/darray.cpp
    ${SOURCES_DIR}/audiosource/speech/klatt.cpp
    ${SOURCES_DIR}/audiosource/speech/resonator.cpp
    ${SOURCES_DIR}/audiosource/speech/soloud_speech.cpp
    ${SOURCES_DIR}/audiosource/speech/tts.cpp
    ${SOURCES_DIR}/audiosource/tedsid/sid.cpp
    ${SOURCES_DIR}/audiosource/tedsid/soloud_tedsid.cpp
    ${SOURCES_DIR}/audiosource/tedsid/ted.cpp
    ${SOURCES_DIR}/audiosource/vic/soloud_vic.cpp
    ${SOURCES_DIR}/audiosource/vizsn/soloud_vizsn.cpp
    ${SOURCES_DIR}/audiosource/wav/dr_impl.cpp
    ${SOURCES_DIR}/audiosource/wav/soloud_wav.cpp
    ${SOURCES_DIR}/audiosource/wav/soloud_wavstream.cpp
    ${SOURCES_DIR}/audiosource/wav/stb_vorbis.c
)

# Filter sources
set(FILTER_SOURCES
    ${SOURCES_DIR}/filter/soloud_bassboostfilter.cpp
    ${SOURCES_DIR}/filter/soloud_biquadresonantfilter.cpp
    ${SOURCES_DIR}/filter/soloud_dcremovalfilter.cpp
    ${SOURCES_DIR}/filter/soloud_echofilter.cpp
    ${SOURCES_DIR}/filter/soloud_fftfilter.cpp
    ${SOURCES_DIR}/filter/soloud_flangerfilter.cpp
    ${SOURCES_DIR}/filter/soloud_freeverbfilter.cpp
    ${SOURCES_DIR}/filter/soloud_lofifilter.cpp
    ${SOURCES_DIR}/filter/soloud_robotizefilter.cpp
    ${SOURCES_DIR}/filter/soloud_waveshaperfilter.cpp
)

# Backend sources and libraries
set(BACKEND_SOURCES)
set(BACKEND_LIBRARIES)
set(BACKEND_DEFINITIONS)

if(SOLOUD_BACKEND_NULL)
    list(APPEND BACKEND_SOURCES ${SOURCES_DIR}/backend/null/soloud_null.cpp)
    list(APPEND BACKEND_DEFINITIONS WITH_NULL)
endif()

if(SOLOUD_BACKEND_SDL2)
    find_package(SDL2 REQUIRED)
    list(APPEND BACKEND_SOURCES ${SOURCES_DIR}/backend/sdl2_static/soloud_sdl2_static.cpp)
    list(APPEND BACKEND_DEFINITIONS WITH_SDL2_STATIC)
    list(APPEND BACKEND_LIBRARIES ${SDL2_LIBRARY})
    include_directories(${SDL2_INCLUDE_DIR})
endif()

if(SOLOUD_BACKEND_ALSA)
    find_library(ALSA_LIBRARY asound)
    if(ALSA_LIBRARY)
        list(APPEND BACKEND_SOURCES ${SOURCES_DIR}/backend/alsa/soloud_alsa.cpp)
        list(APPEND BACKEND_DEFINITIONS WITH_ALSA)
        list(APPEND BACKEND_LIBRARIES ${ALSA_LIBRARY})
    else()
        message(WARNING "ALSA library not found, disabling ALSA backend")
        set(SOLOUD_BACKEND_ALSA OFF)
    endif()
endif()

if(SOLOUD_BACKEND_COREAUDIO)
    if(APPLE)
        find_library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox)
        if(AUDIOTOOLBOX_FRAMEWORK)
            list(APPEND BACKEND_SOURCES ${SOURCES_DIR}/backend/coreaudio/soloud_coreaudio.cpp)
            list(APPEND BACKEND_DEFINITIONS WITH_COREAUDIO)
            list(APPEND BACKEND_LIBRARIES ${AUDIOTOOLBOX_FRAMEWORK})
        endif()
    else()
        message(WARNING "CoreAudio backend can only be enabled on Apple platforms")
        set(SOLOUD_BACKEND_COREAUDIO OFF)
    endif()
endif()

if(SOLOUD_BACKEND_OPENSLES)
    find_library(OPENSLES_LIBRARY OpenSLES)
    if(OPENSLES_LIBRARY)
        list(APPEND BACKEND_SOURCES ${SOURCES_DIR}/backend/opensles/soloud_opensles.cpp)
        list(APPEND BACKEND_DEFINITIONS WITH_OPENSLES)
        list(APPEND BACKEND_LIBRARIES ${OPENSLES_LIBRARY})
    endif()
endif()

if(SOLOUD_BACKEND_XAUDIO2)
    list(APPEND BACKEND_SOURCES ${SOURCES_DIR}/backend/xaudio2/soloud_xaudio2.cpp)
    list(APPEND BACKEND_DEFINITIONS WITH_XAUDIO2)
endif()

if(SOLOUD_BACKEND_WINMM)
    list(APPEND BACKEND_SOURCES ${SOURCES_DIR}/backend/winmm/soloud_winmm.cpp)
    list(APPEND BACKEND_DEFINITIONS WITH_WINMM)
endif()

if(SOLOUD_BACKEND_WASAPI)
    list(APPEND BACKEND_SOURCES ${SOURCES_DIR}/backend/wasapi/soloud_wasapi.cpp)
    list(APPEND BACKEND_DEFINITIONS WITH_WASAPI)
endif()

# C API (optional)
if(SOLOUD_C_API)
    list(APPEND CORE_SOURCES
        ${SOURCES_DIR}/c_api/soloud_c.cpp
    )
endif()

# All sources
set(ALL_SOURCES
    ${CORE_SOURCES}
    ${AUDIOSOURCE_SOURCES}
    ${FILTER_SOURCES}
    ${BACKEND_SOURCES}
)

# Create library
if(SOLOUD_STATIC)
    add_library(soloud STATIC ${ALL_SOURCES})
    set_target_properties(soloud PROPERTIES
        OUTPUT_NAME soloud
        VERSION ${PROJECT_VERSION}
    )
endif()

if(SOLOUD_DYNAMIC)
    add_library(soloud_shared SHARED ${ALL_SOURCES})
    set_target_properties(soloud_shared PROPERTIES
        OUTPUT_NAME soloud
        VERSION ${PROJECT_VERSION}
    )
    target_compile_definitions(soloud_shared PRIVATE SOLOUD_DYNAMIC)
endif()

# Apply definitions and link libraries
if(SOLOUD_STATIC)
    target_compile_definitions(soloud PRIVATE ${BACKEND_DEFINITIONS})
    target_link_libraries(soloud PRIVATE ${BACKEND_LIBRARIES})

    if(MSVC)
        target_compile_definitions(soloud PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
endif()

if(SOLOUD_DYNAMIC)
    target_compile_definitions(soloud_shared PRIVATE ${BACKEND_DEFINITIONS})
    target_link_libraries(soloud_shared PRIVATE ${BACKEND_LIBRARIES})

    if(MSVC)
        target_compile_definitions(soloud_shared PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
endif()

# Install
install(TARGETS soloud
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)
