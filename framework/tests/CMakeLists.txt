project(frameworkUintTest)
cmake_minimum_required(VERSION 3.15)

if (!HAVE_COVERAGE_CONFIG)
    include(../code_coverage.cmake)
endif ()

include(GoogleTest.cmake)
add_subdirectory(dataSource)
if (!TRAVIS)
    add_subdirectory(render)
endif ()
add_subdirectory(demuxer)
add_subdirectory(decoder)

enable_testing()

add_test(
        NAME dataSourceTest
        COMMAND $<TARGET_FILE:dataSourceTest>
)
if (!TRAVIS)
    add_test(
            NAME renderTest
            COMMAND $<TARGET_FILE:renderTest>
    )
endif ()
add_test(
        NAME demuxerUnitTest
        COMMAND $<TARGET_FILE:demuxerUnitTest>
)

add_test(
        NAME decoderUnitTest
        COMMAND $<TARGET_FILE:decoderUnitTest>
)