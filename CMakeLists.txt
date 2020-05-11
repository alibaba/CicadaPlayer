cmake_minimum_required(VERSION 3.15)
project(CicadaMedia)
# Code Coverage Configuration
option(CODE_COVERAGE "Enable coverage reporting" OFF)
option(USEASAN "Enable Address Sanitizer" OFF)
option(USETSAN "Enable Thread Sanitizer" OFF)
option(USEUBSAN "Enable Undefined Behavior Sanitizer" OFF)


set(USEASAN ON)

set(CMDLINE_BUILD ON)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(TARGET_PLATFORM macOSX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(TARGET_PLATFORM Linux)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(TARGET_PLATFORM windows)
    set(USEASAN OFF)
endif ()
if (XCODE)
    set(USEASAN OFF)
endif ()
add_subdirectory(cmdline)

include(framework/${TARGET_PLATFORM}.cmake)
if (BUILD_TEST)
    add_subdirectory(mediaPlayer/tests)
    add_subdirectory(framework/tests)
endif ()
