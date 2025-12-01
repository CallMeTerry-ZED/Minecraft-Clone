cmake_minimum_required(VERSION 3.15)

project(yojimbo VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Options
option(YOJIMBO_BUILD_TESTS "Build test programs" OFF)
option(YOJIMBO_USE_SYSTEM_SODIUM "Use system libsodium instead of bundled" OFF)

# Include directories
set(YOJIMBO_INCLUDE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/sodium
        ${CMAKE_CURRENT_SOURCE_DIR}/tlsf
        ${CMAKE_CURRENT_SOURCE_DIR}/netcode
        ${CMAKE_CURRENT_SOURCE_DIR}/reliable
        ${CMAKE_CURRENT_SOURCE_DIR}/serialize
)

# ============================================================================
# Sodium (crypto library)
# ============================================================================

if(WIN32 OR YOJIMBO_USE_SYSTEM_SODIUM)
    # On Windows, use bundled sodium. On Linux/Mac, try system libsodium first
    if(UNIX AND NOT APPLE AND YOJIMBO_USE_SYSTEM_SODIUM)
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(SODIUM QUIET libsodium)
        endif()
        if(SODIUM_FOUND)
            message(STATUS "yojimbo: Using system libsodium")
        else()
            find_library(SODIUM_LIBRARY NAMES sodium)
            if(SODIUM_LIBRARY)
                set(SODIUM_FOUND TRUE)
                set(SODIUM_LIBRARIES ${SODIUM_LIBRARY})
                set(SODIUM_INCLUDE_DIRS "")
            endif()
        endif()
    endif()

    if(NOT SODIUM_FOUND)
        # Use bundled sodium
        message(STATUS "yojimbo: Using bundled sodium")

        # Get all sodium C files
        file(GLOB_RECURSE SODIUM_SOURCES
                "${CMAKE_CURRENT_SOURCE_DIR}/sodium/*.c"
        )

        # Exclude dummy.c if it exists
        list(FILTER SODIUM_SOURCES EXCLUDE REGEX ".*dummy\\.c$")

        # Get assembly files for non-Windows platforms
        if(UNIX AND NOT APPLE)
            file(GLOB SODIUM_ASM_SOURCES
                    "${CMAKE_CURRENT_SOURCE_DIR}/sodium/*.S"
            )
            if(SODIUM_ASM_SOURCES)
                list(APPEND SODIUM_SOURCES ${SODIUM_ASM_SOURCES})
            endif()
        endif()

        add_library(sodium-builtin STATIC ${SODIUM_SOURCES})
        target_include_directories(sodium-builtin PUBLIC
                ${CMAKE_CURRENT_SOURCE_DIR}/sodium
        )

        # Platform-specific compiler options for sodium
        if(UNIX AND NOT APPLE)
            target_compile_options(sodium-builtin PRIVATE
                    -Wno-unused-parameter
                    -Wno-unused-function
                    -Wno-unknown-pragmas
                    -Wno-unused-variable
                    -Wno-type-limits
            )
        endif()

        if(MSVC)
            target_compile_definitions(sodium-builtin PRIVATE
                    _CRT_SECURE_NO_WARNINGS
            )
        endif()

        set(SODIUM_TARGET sodium-builtin)
    else()
        # Use system sodium
        add_library(sodium-builtin INTERFACE)
        target_link_libraries(sodium-builtin INTERFACE ${SODIUM_LIBRARIES})
        if(SODIUM_INCLUDE_DIRS)
            target_include_directories(sodium-builtin INTERFACE ${SODIUM_INCLUDE_DIRS})
        endif()
        set(SODIUM_TARGET sodium-builtin)
    endif()
else()
    # Linux/Mac: prefer system libsodium
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(SODIUM QUIET libsodium)
    endif()

    if(NOT SODIUM_FOUND)
        find_library(SODIUM_LIBRARY NAMES sodium)
        if(SODIUM_LIBRARY)
            set(SODIUM_FOUND TRUE)
            set(SODIUM_LIBRARIES ${SODIUM_LIBRARY})
        endif()
    endif()

    if(SODIUM_FOUND)
        message(STATUS "yojimbo: Using system libsodium")
        add_library(sodium-builtin INTERFACE)
        target_link_libraries(sodium-builtin INTERFACE ${SODIUM_LIBRARIES})
        if(SODIUM_INCLUDE_DIRS)
            target_include_directories(sodium-builtin INTERFACE ${SODIUM_INCLUDE_DIRS})
        endif()
        set(SODIUM_TARGET sodium-builtin)
    else()
        message(WARNING "yojimbo: System libsodium not found, using bundled version")
        # Fall back to bundled
        file(GLOB_RECURSE SODIUM_SOURCES
                "${CMAKE_CURRENT_SOURCE_DIR}/sodium/*.c"
        )
        list(FILTER SODIUM_SOURCES EXCLUDE REGEX ".*dummy\\.c$")

        file(GLOB SODIUM_ASM_SOURCES
                "${CMAKE_CURRENT_SOURCE_DIR}/sodium/*.S"
        )
        if(SODIUM_ASM_SOURCES)
            list(APPEND SODIUM_SOURCES ${SODIUM_ASM_SOURCES})
        endif()

        add_library(sodium-builtin STATIC ${SODIUM_SOURCES})
        target_include_directories(sodium-builtin PUBLIC
                ${CMAKE_CURRENT_SOURCE_DIR}/sodium
        )
        target_compile_options(sodium-builtin PRIVATE
                -Wno-unused-parameter
                -Wno-unused-function
                -Wno-unknown-pragmas
                -Wno-unused-variable
                -Wno-type-limits
        )
        set(SODIUM_TARGET sodium-builtin)
    endif()
endif()

# ============================================================================
# netcode library
# ============================================================================

add_library(netcode STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/netcode/netcode.c
        ${CMAKE_CURRENT_SOURCE_DIR}/netcode/netcode.h
)

target_include_directories(netcode PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/netcode
)

target_compile_definitions(netcode PRIVATE
        NETCODE_ENABLE_TESTS=1
)

if(MSVC)
    target_compile_definitions(netcode PRIVATE
            _CRT_SECURE_NO_WARNINGS
    )
endif()

# ============================================================================
# reliable library
# ============================================================================

add_library(reliable STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/reliable/reliable.c
        ${CMAKE_CURRENT_SOURCE_DIR}/reliable/reliable.h
)

target_include_directories(reliable PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/reliable
)

target_compile_definitions(reliable PRIVATE
        RELIABLE_ENABLE_TESTS=1
)

if(MSVC)
    target_compile_definitions(reliable PRIVATE
            _CRT_SECURE_NO_WARNINGS
    )
endif()

# ============================================================================
# tlsf library (memory allocator)
# ============================================================================

add_library(tlsf STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/tlsf/tlsf.c
        ${CMAKE_CURRENT_SOURCE_DIR}/tlsf/tlsf.h
)

target_include_directories(tlsf PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/tlsf
)

if(MSVC)
    target_compile_definitions(tlsf PRIVATE
            _CRT_SECURE_NO_WARNINGS
    )
endif()

# ============================================================================
# yojimbo library (main library)
# ============================================================================

# Get all source files
file(GLOB YOJIMBO_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
)

add_library(yojimbo STATIC
        ${YOJIMBO_SOURCES}
)

target_include_directories(yojimbo PUBLIC
        ${YOJIMBO_INCLUDE_DIRS}
)

# Link dependencies
target_link_libraries(yojimbo PUBLIC
        ${SODIUM_TARGET}
        netcode
        reliable
        tlsf
)

# Compiler-specific options
if(MSVC)
    target_compile_definitions(yojimbo PRIVATE
            _CRT_SECURE_NO_WARNINGS
    )
    target_compile_options(yojimbo PRIVATE
            /W4
            /WX-
    )
else()
    target_compile_options(yojimbo PRIVATE
            -Wall
            -Wextra
            -Wno-unused-parameter
    )
endif()

# Debug/Release definitions
target_compile_definitions(yojimbo PRIVATE
        $<$<CONFIG:Debug>:YOJIMBO_DEBUG>
        $<$<CONFIG:Debug>:NETCODE_DEBUG>
        $<$<CONFIG:Debug>:RELIABLE_DEBUG>
        $<$<CONFIG:Release>:YOJIMBO_RELEASE>
        $<$<CONFIG:Release>:NETCODE_RELEASE>
        $<$<CONFIG:Release>:RELIABLE_RELEASE>
)

# Disable RTTI
set_target_properties(yojimbo PROPERTIES
        CXX_RTTI OFF
)

# ============================================================================
# Install (optional)
# ============================================================================

install(TARGETS yojimbo netcode reliable tlsf ${SODIUM_TARGET}
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h"
)

# Also install headers from subdirectories
install(FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/netcode/netcode.h
        ${CMAKE_CURRENT_SOURCE_DIR}/reliable/reliable.h
        ${CMAKE_CURRENT_SOURCE_DIR}/tlsf/tlsf.h
        ${CMAKE_CURRENT_SOURCE_DIR}/serialize/serialize.h
        DESTINATION include
)