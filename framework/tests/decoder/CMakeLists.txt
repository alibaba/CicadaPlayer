cmake_minimum_required(VERSION 3.6)
project(decoderUnitTest LANGUAGES CXX)

# require C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0079 NEW)
add_executable(decoderUnitTest "")

if (APPLE)
    include(../Apple.cmake)
endif ()

include(../../${TARGET_PLATFORM}.cmake)
target_sources(decoderUnitTest
        PRIVATE
        decoderUnitTest.cpp
        )

target_include_directories(
        decoderUnitTest
        PRIVATE
        ../../
)

target_link_libraries(
        decoderUnitTest PRIVATE
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
        gtest_main
        ${FRAMEWORK_LIBS})

target_link_directories(decoderUnitTest PRIVATE ${COMMON_LIB_DIR})

if (APPLE)
    target_link_libraries(
            decoderUnitTest PUBLIC
            iconv
            bz2
            ${FRAMEWORK_LIBS}
    )
else ()
    target_link_libraries(
            decoderUnitTest PUBLIC
            dl
            ssl
            crypto
            pthread
    )

endif ()
if (HAVE_COVERAGE_CONFIG)
    target_link_libraries(decoderUnitTest PUBLIC coverage_config)
endif ()