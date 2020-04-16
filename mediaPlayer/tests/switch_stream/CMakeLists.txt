cmake_minimum_required(VERSION 3.15)
project(mediaPlayerSwitchStream)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (!HAVE_COVERAGE_CONFIG)
    include(../../../framework/code_coverage.cmake)
endif ()

if (APPLE)
    include(../../../framework/tests/Apple.cmake)
endif ()

include(../../../framework/${TARGET_PLATFORM}.cmake)

add_executable(mediaPlayerSwitchStreamTest "")

target_sources(mediaPlayerSwitchStreamTest
        PRIVATE
        mediaPlayerSwitchStream.cpp
        ../mediaPlayerTest.cpp
        ../player_command.cpp
        )

target_include_directories(mediaPlayerSwitchStreamTest PRIVATE ../..)

target_link_libraries(mediaPlayerSwitchStreamTest PRIVATE
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
        ${FRAMEWORK_LIBS}
        gtest_main)

target_link_directories(mediaPlayerSwitchStreamTest PRIVATE
        ../../../external/install/ffmpeg/${CMAKE_SYSTEM_NAME}/x86_64/lib
        ../../../external/install/curl/${CMAKE_SYSTEM_NAME}/x86_64/lib
        ../../../external/install/openssl/${CMAKE_SYSTEM_NAME}/x86_64/lib)

if (ENABLE_SDL)
    target_link_libraries(mediaPlayerSwitchStreamTest PUBLIC
            SDL2
            )
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(mediaPlayerSwitchStreamTest PUBLIC
            bcrypt
            )
else ()
    target_link_libraries(mediaPlayerSwitchStreamTest PUBLIC
            z
            dl
            )
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(mediaPlayerSwitchStreamTest PUBLIC
            bz2
            iconv
            )
endif ()
if (APPLE)
    target_link_libraries(
            mediaPlayerSwitchStreamTest PUBLIC
            iconv
            bz2
            ${FRAMEWORK_LIBS}
    )
else ()
    target_link_libraries(
            mediaPlayerSwitchStreamTest PUBLIC
            dl
            ssl
            crypto
            pthread
    )
endif ()

if (HAVE_COVERAGE_CONFIG)
    target_link_libraries(mediaPlayerSwitchStreamTest PUBLIC coverage_config)
endif ()

