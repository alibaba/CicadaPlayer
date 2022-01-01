cmake_minimum_required(VERSION 3.6)
project(framework_drm)
set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES
        DrmInfo.cpp
        DrmManager.cpp
        DrmHandler.cpp
        DrmHandlerPrototype.cpp
        )

if (ANDROID)
    list(APPEND SOURCE_FILES
            WideVineDrmHandler.cpp
            )
endif ()


include_directories(
        ${COMMON_INC_DIR})
add_library(framework_drm ${TARGET_LIBRARY_TYPE} ${SOURCE_FILES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_CONSTANT_MACROS")

if (HAVE_COVERAGE_CONFIG)
    target_link_libraries(framework_drm PUBLIC coverage_config)
endif ()