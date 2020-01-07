cmake_minimum_required(VERSION 3.6)
project(ffmpegWrape)
set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES)

include_directories(${COMMON_INC_DIR} ${FFMPEG_SOURCE_DIR})

add_library(ffmpegWrape STATIC ${SOURCE_FILES})