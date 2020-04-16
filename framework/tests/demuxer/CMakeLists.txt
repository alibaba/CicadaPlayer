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

include(../../${TARGET_PLATFORM}.cmake)

target_sources(demuxerUnitTest
        PRIVATE
        demuxerUnitTest.cpp
        )

target_include_directories(
        demuxerUnitTest
        PRIVATE
        ../../
        ${COMMON_INC_DIR}
)

target_link_libraries(
        demuxerUnitTest PRIVATE
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
        gtest_main)

target_link_directories(demuxerUnitTest PRIVATE ${COMMON_LIB_DIR})

if (APPLE)
    target_link_libraries(
            demuxerUnitTest PUBLIC
            iconv
            bz2
            ${FRAMEWORK_LIBS}
    )
else ()
    target_link_libraries(
            demuxerUnitTest PUBLIC
            dl
            ssl
            crypto
            pthread
    )

endif ()
if (HAVE_COVERAGE_CONFIG)
    target_link_libraries(demuxerUnitTest PUBLIC coverage_config)
endif ()