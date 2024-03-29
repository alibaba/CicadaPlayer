cmake_minimum_required(VERSION 3.6)
project(framework_filter)
set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES IAudioFilter.cpp
        ffmpegAudioFilter.cpp filterFactory.cpp filterFactory.h)

if(ENABLE_VIDEO_FILTER)
set(SOURCE_FILES ${SOURCE_FILES}
        IVideoFilter.h
        ffmpegVideoFilter.cpp
        ffmpegVideoFilter.h
        filterPrototype.cpp
        filterPrototype.h
        FilterManager.h
        FilterManager.cpp
        VideoFilterChain.cpp
        VideoFilterChain.h
        )
endif()

include_directories(
        ${COMMON_INC_DIR})
add_library(framework_filter ${TARGET_LIBRARY_TYPE} ${SOURCE_FILES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__STDC_CONSTANT_MACROS")

if (HAVE_COVERAGE_CONFIG)
    target_link_libraries(framework_filter PUBLIC coverage_config)
endif ()