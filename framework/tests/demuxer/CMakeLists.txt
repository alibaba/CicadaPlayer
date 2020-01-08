cmake_minimum_required(VERSION 3.6)
project(demuxerUnitTest LANGUAGES CXX)

# require C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0079 NEW)
add_executable(demuxerUnitTest "" demuxerUtils.cpp demuxerUtils.h)

if (APPLE)
    include(../Apple.cmake)
endif ()

target_sources(demuxerUnitTest
        PRIVATE
        demuxerUnitTest.cpp
        )

target_include_directories(
        demuxerUnitTest
        PRIVATE
)

target_link_libraries(
        demuxerUnitTest
        demuxer
        videodec
        data_source
        #       plugin
        framework_utils
        avformat
        avcodec
        swresample
        avutil
        swscale
        z
        curl
        ssl
        crypto
        gtest_main)

if (APPLE)
    target_link_libraries(
            demuxerUnitTest
            iconv
            bz2
            ${FRAMEWORK_LIBS}
    )
else()
    target_link_libraries(
            demuxerUnitTest
            dl
            pthread
    )

endif ()