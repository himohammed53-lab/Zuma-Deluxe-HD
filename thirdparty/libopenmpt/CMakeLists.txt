# Minimal static build of libopenmpt (release sources) for Zuma HD.
# Only what is needed to play zuma.mo3 (MO3 with Vorbis-compressed samples).
cmake_minimum_required(VERSION 3.16)

file(GLOB LIBOPENMPT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/common/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/soundlib/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/soundlib/plugins/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/soundlib/plugins/dmo/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/sounddsp/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/libopenmpt/libopenmpt_c.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/libopenmpt/libopenmpt_cxx.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/libopenmpt/libopenmpt_ext_impl.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/libopenmpt/libopenmpt_impl.cpp"
)

add_library(libopenmpt STATIC ${LIBOPENMPT_SOURCES})

set_target_properties(libopenmpt PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(libopenmpt
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/common"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_definitions(libopenmpt
    PRIVATE
        LIBOPENMPT_BUILD
        MPT_WITH_OGG
        MPT_WITH_VORBIS
        MPT_WITH_VORBISFILE
)

target_link_libraries(libopenmpt PRIVATE vorbisfile vorbis ogg)

if(MSVC)
    target_compile_options(libopenmpt PRIVATE /bigobj /utf-8)
endif()
