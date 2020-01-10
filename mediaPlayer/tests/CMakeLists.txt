cmake_minimum_required(VERSION 3.15)
project(mediaPlayerTest)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (!HAVE_COVERAGE_CONFIG)
    include(../../framework/code_coverage.cmake)
endif ()

if (APPLE)
    include(../../framework/tests/Apple.cmake)
endif ()

include(../../framework/tests/GoogleTest.cmake)

add_executable(mediaPlayerTest "")

target_sources(mediaPlayerTest
        PRIVATE
        mediaPlayerUnitTest.cpp
        mediaPlayerTest.cpp
        )

target_include_directories(mediaPlayerTest PRIVATE ../)

target_link_libraries(mediaPlayerTest PRIVATE
        media_player
        demuxer
        data_source
        cacheModule
        muxer
        render
        videodec
        framework_filter
        framework_utils
        avfilter
        avformat
        avcodec
        swresample
        avutil
        curl
        ssl
        crypto
        pthread
        ${FRAMEWORK_LIBS}
        gtest_main)

target_link_directories(mediaPlayerTest PRIVATE
        ../../external/install/ffmpeg/${CMAKE_SYSTEM_NAME}/x86_64/lib
        ../../external/install/curl/${CMAKE_SYSTEM_NAME}/x86_64/lib
        ../../external/install/openssl/${CMAKE_SYSTEM_NAME}/x86_64/lib)

if (ENABLE_SDL)
    target_link_libraries(mediaPlayerTest PUBLIC
            SDL2
            )
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(mediaPlayerTest PUBLIC
            bcrypt
            )
else ()
    target_link_libraries(mediaPlayerTest PUBLIC
            z
            dl
            )
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(mediaPlayerTest PUBLIC
            bz2
            iconv
            )
endif ()

if (HAVE_COVERAGE_CONFIG)
    target_link_libraries(mediaPlayerTest PUBLIC coverage_config)
endif ()

enable_testing()

add_test(
        NAME mediaPlayerTest
        COMMAND $<TARGET_FILE:mediaPlayerTest>
)
