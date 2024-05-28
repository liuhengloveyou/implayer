cmake_minimum_required(VERSION 3.21)
project(implayer VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

#set(CMAKE_BUILD_TYPE "Release")

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    add_compile_definitions(_ITERATOR_DEBUG_LEVEL=0)
endif ()

set(CMAKE_VERBOSE_MAKEFILE ON)

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/deps/imgui)
set(FFMPEG_DIR ${CMAKE_SOURCE_DIR}/../ffmpeg-6.1.1-full_build-shared)

########### 三方依赖库 ############
include(${CMAKE_SOURCE_DIR}/deps/sdl2.cmake)
########### 三方依赖库 ###########

find_library(LIBAVUTIL avutil ${FFMPEG_DIR}/lib)
find_library(LIBAVCODEC avcodec ${FFMPEG_DIR}/lib)
find_library(LIBAVDEVICE avdevice ${FFMPEG_DIR}/lib)
find_library(LIBAVFILTER avfilter ${FFMPEG_DIR}/lib)
find_library(LIBAVFORMAT avformat ${FFMPEG_DIR}/lib)
find_library(LIBSWSCALE swscale ${FFMPEG_DIR}/lib)
find_library(LIBSWRESAMPLE swresample ${FFMPEG_DIR}/lib)

set(DEPS_LIB
        SDL2::SDL2main 
        SDL2::SDL2-static
        ${LIBAVUTIL}
        ${LIBAVCODEC}
        ${LIBAVFORMAT}
        ${LIBSWRESAMPLE}
        ${LIBSWSCALE}
)

set(SOURCES
        ${SOURCE_DIR}/main.cc
        ${SOURCE_DIR}/core/player.cc
        ${SOURCE_DIR}/utils/thread_base.cc
        ${SOURCE_DIR}/source/ffmpeg_decoder.cc
        ${SOURCE_DIR}/source/simple_source.cc
        ${SOURCE_DIR}/output/base_video_output.cc
        ${SOURCE_DIR}/output/base_audio_output.cc
        ${SOURCE_DIR}/output/sdl2_video_output.cc
        ${SOURCE_DIR}/output/sdl2_audio_output.cc
        ${SOURCE_DIR}/output/sdl2_video_output.cc
        ${SOURCE_DIR}/output/imgui_sdl2_video_output.cc
        ${SOURCE_DIR}/demo/avio_write_h264.cc
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
        ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

set(INCLUDES
        ${CMAKE_SOURCE_DIR}/deps
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/src
        ${SOURCE_DIR}
        ${FFMPEG_DIR}/include
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends 
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDES})
target_link_libraries(${PROJECT_NAME} ${DEPS_LIB})

