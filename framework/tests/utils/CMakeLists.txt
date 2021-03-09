cmake_minimum_required(VERSION 3.6)
project(utils_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

if (APPLE)
    include(../Apple.cmake)
endif ()
include(../../${TARGET_PLATFORM}.cmake)
add_executable(utils_test utils_test.cpp)
target_link_libraries(utils_test PRIVATE
        framework_utils
        gtest_main)

target_include_directories(
        utils_test
        PRIVATE
        ../../
        ${COMMON_INC_DIR}
)


