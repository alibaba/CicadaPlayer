cmake_minimum_required(VERSION 3.6)
project(dataSourceTest LANGUAGES CXX)

# require C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE)
        include(../Apple.cmake)
endif ()

include(../../${TARGET_PLATFORM}.cmake)

cmake_policy(SET CMP0079 NEW)
add_executable(dataSourceTest "")

target_sources(dataSourceTest
        PRIVATE
        dataSourceUnitTest.cpp
        )

target_include_directories(
        dataSourceTest
        PRIVATE
        ../../../plugin
        ../../
        ${COMMON_INC_DIR}
)

target_link_libraries(dataSourceTest PRIVATE
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


target_link_directories(dataSourceTest PRIVATE ${COMMON_LIB_DIR})
if (APPLE)
        target_link_libraries(
                dataSourceTest PUBLIC
                iconv
                bz2
                ${FRAMEWORK_LIBS}
        )
else ()
        target_link_libraries(
                dataSourceTest PUBLIC
                dl
                ssl
                crypto
                pthread
        )

endif ()
if (HAVE_COVERAGE_CONFIG)
        target_link_libraries(dataSourceTest PUBLIC coverage_config)
endif ()