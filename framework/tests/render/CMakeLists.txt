cmake_minimum_required(VERSION 3.6)
project(renderTest LANGUAGES CXX)

# require C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0079 NEW)
add_executable(renderTest "" renderTest.cpp)

if (APPLE)
    include(../Apple.cmake)
endif ()

include(../../${TARGET_PLATFORM}.cmake)

target_sources(renderTest
        PRIVATE

        )

target_include_directories(
        dataSourceTest
        PRIVATE
)

target_link_libraries(renderTest PRIVATE
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

target_link_directories(renderTest PRIVATE ${COMMON_LIB_DIR})

if (APPLE)
    target_link_libraries(
            renderTest PUBLIC
            iconv
            bz2
            ${FRAMEWORK_LIBS}
    )
else ()
    target_link_libraries(
            renderTest PUBLIC
            dl
            pthread
    )

endif ()
if (HAVE_COVERAGE_CONFIG)
    target_link_libraries(renderTest PUBLIC coverage_config)
endif ()

