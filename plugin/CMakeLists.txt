cmake_minimum_required(VERSION 3.6)
project(plugin)

set(CMAKE_CXX_STANDARD 11)

if (IOS)
    message("build ios")
    set(TARGET_PLATFORM iOS)
endif ()

if (ANDROID)
    set(TARGET_PLATFORM Android)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES Windows)
    message("build windows")
    set(TARGET_PLATFORM windows)
elseif (${CMAKE_SYSTEM_NAME} MATCHES Emscripten)
    message("build Emscripten")
    set(TARGET_PLATFORM Emscripten)
endif ()

if (TARGET_PLATFORM)
else ()
    set(TARGET_PLATFORM macOSX)
endif ()
message("TARGET_PLATFORM is ${TARGET_PLATFORM}")

set(TOPDIR ${PROJECT_SOURCE_DIR}/..)
include(../framework/${TARGET_PLATFORM}.cmake)

set(SOURCE_FILES
        BiDataSource.cpp
        BiDataSource.h
        )

add_library(plugin ${TARGET_LIBRARY_TYPE} ${SOURCE_FILES})

target_include_directories(plugin
        PUBLIC ../framework ${COMMON_INC_DIR})