cmake_minimum_required(VERSION 3.6)
project(dataSourceTest LANGUAGES CXX)

# require C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE)
    include(../Apple.cmake)
endif ()
include(../../${TARGET_PLATFORM}.cmake)

add_executable(communicationTest test.cpp)
target_link_libraries(communicationTest PRIVATE
        communication
        framework_utils
        avformat
        avcodec
        avutil
        )
target_include_directories(communicationTest PRIVATE
        ../../
        ${COMMON_INC_DIR}
        )

target_link_directories(communicationTest PRIVATE ${COMMON_LIB_DIR})

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(communicationTest PUBLIC
            bcrypt
            )
else ()
    target_link_libraries(communicationTest PUBLIC
            z
            dl
            )
endif ()
if (APPLE)
    target_link_libraries(
            communicationTest PUBLIC
            iconv
            bz2
            ${FRAMEWORK_LIBS}
    )
else ()
    target_link_libraries(
            communicationTest PUBLIC
            dl
            ssl
            crypto
            pthread
    )
endif ()