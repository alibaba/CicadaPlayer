cmake_minimum_required(VERSION 3.6)
project(WebAssemblyPlayer)
set(CMAKE_CXX_STANDARD 11)

set(SRC_FILES
        bitStreamDecoder.cpp
        bitStreamDecoder.h
        WABitStreamDecoder.cpp
        WABitStreamDecoder.h
        )

add_library(bitStreamDecoder ${SRC_FILES})
target_include_directories(bitStreamDecoder PRIVATE ../
        ${COMMON_INC_DIR})

