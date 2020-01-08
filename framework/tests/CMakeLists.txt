project(frameworkUintTest)
cmake_minimum_required(VERSION 3.15)

include(GoogleTest.cmake)
add_subdirectory(dataSource)
add_subdirectory(render)
add_subdirectory(demuxer)
add_subdirectory(decoder)

enable_testing()

add_test(
        NAME dataSourceTest
        COMMAND $<TARGET_FILE:dataSourceTest>
)

add_test(
        NAME renderTest
        COMMAND $<TARGET_FILE:renderTest>
)

add_test(
        NAME demuxerUnitTest
        COMMAND $<TARGET_FILE:demuxerUnitTest>
)

add_test(
        NAME decoderUnitTest
        COMMAND $<TARGET_FILE:decoderUnitTest>
)