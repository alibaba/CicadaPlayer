cmake_minimum_required(VERSION 3.6)
project(codec)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        IVideoFrame.h
        IDecoder.h
        codecPrototype.cpp
        codecPrototype.h
        videoDecoderFactory.cpp
        decoderFactory.cpp
        decoderFactory.h
        ActiveDecoder.cpp
        ActiveDecoder.h
        avcodecDecoder.cpp
        avcodecDecoder.h)
if (ANDROID)
    set(SOURCE_FILES ${SOURCE_FILES}
            Android/mediaCodec.h
            Android/mediaCodec.cpp
            Android/mediacodec_jni.h
            Android/mediacodec_jni.cpp
            Android/mediaCodecDecoder.h
            Android/mediaCodecDecoder.cpp
            )
endif ()

if (APPLE)
    list(APPEND SOURCE_FILES
            Apple/AppleVideoToolBox.cpp
            Apple/AppleVideoToolBox.h
            Apple/video_tool_box_utils.c
            )
endif ()

if (IOS)
    list(APPEND SOURCE_FILES
            utils_ios.mm
            )
endif ()


include_directories(
#        ${CICADA_FRAMEWORK_DIR}/inc
#        ${CICADA_FRAMEWORK_INSTALL_BOOST_INC_DIR}
#        ${CICADA_FRAMEWORK_DIR}/src/mdf
#        ${CICADA_FRAMEWORK_DIR}/src
#        ${CICADA_FRAMEWORK_DIR}/src/message
        ${FFMPEG_SOURCE_DIR}
        ${COMMON_INC_DIR})

link_directories(
#        ${CICADA_FRAMEWORK_INSTALL_LIB_DIR}
        ${CICADA_FRAMEWORK_INSTALL_BOOST_LIB_DIR}
        ${COMMON_LIB_DIR})

if (TARGET_PLATFORM STREQUAL "windows")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-inconsistent-missing-override -frtti")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -U_FORTIFY_SOURCE -fstack-protector-all")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U_FORTIFY_SOURCE -fstack-protector-all")
endif ()


link_libraries(${LINNK_LIBRARYS})

add_library(videodec ${TARGET_LIBRARY_TYPE} ${SOURCE_FILES})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")
