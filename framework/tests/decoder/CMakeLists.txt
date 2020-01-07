cmake_minimum_required(VERSION 3.6)
project(decoderUnitTest LANGUAGES CXX)

# require C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0079 NEW)
include(../GoogleTest.cmake)
add_executable(decoderUnitTest "")

if (APPLE)
    include(../Apple.cmake)
endif ()

target_sources(decoderUnitTest
        PRIVATE
        decoderUnitTest.cpp
        )

target_include_directories(
        decoderUnitTest
        PRIVATE
        ../../../plugin
)

target_link_libraries(
        decoderUnitTest
        demuxer
        videodec
        data_source
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
        gtest_main
        ${FRAMEWORK_LIBS})

if (APPLE)
    target_link_libraries(
            decoderUnitTest
            iconv
            bz2
            ${FRAMEWORK_LIBS}
    )
else()
    target_link_libraries(
            decoderUnitTest
            dl
            pthread
    )

endif ()
enable_testing()

add_test(
        NAME google_test
        COMMAND $<TARGET_FILE:decoderUnitTest>
)