cmake_minimum_required(VERSION 3.6)
project(renderTest LANGUAGES CXX)

# require C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0079 NEW)
include(../GoogleTest.cmake)
add_executable(renderTest "" renderTest.cpp)

if (APPLE)
    include(../Apple.cmake)
endif ()

target_sources(renderTest
        PRIVATE

        )

target_include_directories(
        dataSourceTest
        PRIVATE
)

target_link_libraries(
        renderTest
        demuxer
        videodec
        data_source
        render
        framework_utils
        framework_filter
        avformat
        avfilter
        avcodec
        swresample
        avutil
        swscale
        z
        curl
        ssl
        crypto
        ${FRAMEWORK_LIBS}
        gtest_main)

if (APPLE)
    target_link_libraries(
            renderTest
            iconv
            bz2
            ${FRAMEWORK_LIBS}
    )
else()
    target_link_libraries(
            renderTest
            dl
            pthread
    )

endif ()
enable_testing()

add_test(
        NAME google_test
        COMMAND $<TARGET_FILE:dataSourceTest>
)