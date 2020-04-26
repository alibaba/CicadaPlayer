project(frameworkUintTest)
cmake_minimum_required(VERSION 3.15)

if (!HAVE_COVERAGE_CONFIG)
    include(../code_coverage.cmake)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(TARGET_PLATFORM macOSX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(TARGET_PLATFORM Linux)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(TARGET_PLATFORM windows)
endif ()

include(../${TARGET_PLATFORM}.cmake)

include(GoogleTest.cmake)
add_subdirectory(dataSource)
#add_subdirectory(render)
add_subdirectory(demuxer)
add_subdirectory(decoder)
add_subdirectory(communication)

enable_testing()

add_test(
        NAME dataSourceTest
        COMMAND $<TARGET_FILE:dataSourceTest>
)

#add_test(
#        NAME renderTest
#        COMMAND $<TARGET_FILE:renderTest>
#)

add_test(
        NAME demuxerUnitTest
        COMMAND $<TARGET_FILE:demuxerUnitTest>
)

add_test(
        NAME decoderUnitTest
        COMMAND $<TARGET_FILE:decoderUnitTest>
)