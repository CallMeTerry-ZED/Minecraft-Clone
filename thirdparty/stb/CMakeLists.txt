cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)

set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/Thirdparty)
set(SOURCES_DIR ${CMAKE_SOURCE_DIR}/Sources)

set(STB_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_vorbis.c
)

set(STB_INC
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_c_lexer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_connected_components.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_divide.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_ds.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_dxt.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_easy_font.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_herringbone_wang_tile.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_hexwave.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_image.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_image_resize2.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_image_write.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_include.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_leakcheck.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_perlin.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_rect_pack.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_sprintf.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_textedit.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_tilemap_editor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_truetype.h
        ${CMAKE_CURRENT_SOURCE_DIR}/stb_voxel_render.h
)

add_library(STB STATIC
        ${STB_SRC}
        ${STB_INC}
)

target_include_directories(STB PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/
)

target_link_libraries(STB PRIVATE

)

target_compile_definitions(STB PRIVATE
)