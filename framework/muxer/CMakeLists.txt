cmake_minimum_required(VERSION 3.6)
project(muxer)

set(TOPDIR ${PROJECT_SOURCE_DIR}/../..)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        IMuxer.h
        IMuxerPrototype.cpp
        IMuxerPrototype.h
        ffmpegMuxer/FfmpegMuxer.cpp
        ffmpegMuxer/FfmpegMuxer.h
        ffmpegMuxer/MetaToCodec.cpp
        ffmpegMuxer/MetaToCodec.h
        )


include_directories(
#${CICADA_FRAMEWORK_DIR}/inc
 #       ${CICADA_FRAMEWORK_INSTALL_BOOST_INC_DIR}
 #       ${CICADA_FRAMEWORK_DIR}/src/mdf
 #       ${CICADA_FRAMEWORK_DIR}/src
  #      ${CICADA_FRAMEWORK_DIR}/src/message
   #     ${CICADA_FRAMEWORK_DIR}/src/subtitle_parser
        ${FFMPEG_SOURCE_DIR}
        ${COMMON_INC_DIR})

link_directories(
#  ${CICADA_FRAMEWORK_INSTALL_LIB_DIR}
        ${CICADA_FRAMEWORK_INSTALL_BOOST_LIB_DIR}
        ${COMMON_LIB_DIR})

if (TARGET_PLATFORM STREQUAL "windows")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-inconsistent-missing-override")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -U_FORTIFY_SOURCE -fstack-protector-all")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U_FORTIFY_SOURCE -fstack-protector-all")
endif ()

link_libraries(${LINNK_LIBRARYS})

add_library(muxer STATIC ${SOURCE_FILES})
if (HAVE_COVERAGE_CONFIG)
    target_link_libraries(muxer PUBLIC coverage_config)
endif ()