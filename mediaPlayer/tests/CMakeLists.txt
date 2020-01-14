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
add_subdirectory(formatTest)
add_subdirectory(seekTest)
add_subdirectory(apiTest)
add_subdirectory(switch_stream)
add_subdirectory(cache)

enable_testing()

add_test(
        NAME mediaPlayerFormatTest
        COMMAND $<TARGET_FILE:mediaPlayerFormatTest>
)
add_test(
        NAME mediaPlayerSeekTest
        COMMAND $<TARGET_FILE:mediaPlayerSeekTest>
)
add_test(
        NAME mediaPlayerApiTest
        COMMAND $<TARGET_FILE:mediaPlayerApiTest>
)
#add_test(
#        NAME mediaPlayerSwitchStreamTest
#        COMMAND $<TARGET_FILE:mediaPlayerSwitchStreamTest>
#)
add_test(
        NAME mediaPlayerSwitchStreamTest
        COMMAND $<TARGET_FILE:mediaPlayerSwitchStreamTest>
)
add_test(
        NAME mediaPlayerCacheTest
        COMMAND $<TARGET_FILE:mediaPlayerCacheTest>
)