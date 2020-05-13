cmake_minimum_required(VERSION 3.6)
project(codec)

set(CMAKE_CXX_STANDARD 11)

add_library(videodec ${TARGET_LIBRARY_TYPE} "")

target_sources(videodec PRIVATE
        IVideoFrame.h
        IDecoder.h
        codecPrototype.cpp
        codecPrototype.h
        videoDecoderFactory.cpp
        decoderFactory.cpp
        decoderFactory.h
        ActiveDecoder.cpp
        ActiveDecoder.h)

if (ENABLE_AVCODEC_DECODER)
    target_compile_definitions(videodec PRIVATE ENABLE_AVCODEC_DECODER)
    target_sources(videodec PRIVATE
            avcodecDecoder.cpp
            avcodecDecoder.h
            )

endif ()

if (ANDROID AND ENABLE_MEDIA_CODEC_DECODER)
    target_compile_definitions(videodec PRIVATE ENABLE_MEDIA_CODEC_DECODER)
    target_sources(videodec PRIVATE
            Android/mediaCodec.h
            Android/mediaCodec.cpp
            Android/mediacodec_jni.h
            Android/mediacodec_jni.cpp
            Android/mediaCodecDecoder.h
            Android/mediaCodecDecoder.cpp
            )
endif ()

if (APPLE AND ENABLE_VTB_DECODER)
    target_compile_definitions(videodec PRIVATE ENABLE_VTB_DECODER)
    target_sources(videodec PRIVATE
            Apple/AppleVideoToolBox.cpp
            Apple/AppleVideoToolBox.h
            Apple/video_tool_box_utils.c
            )
endif ()

if (IOS)
    target_sources(videodec PRIVATE
            utils_ios.mm
            )
endif ()


target_include_directories(videodec PRIVATE
        ${FFMPEG_SOURCE_DIR}
        ${COMMON_INC_DIR})

link_directories(
        ${CICADA_FRAMEWORK_INSTALL_BOOST_LIB_DIR}
        ${COMMON_LIB_DIR})

if (TARGET_PLATFORM STREQUAL "windows")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-inconsistent-missing-override")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -U_FORTIFY_SOURCE -fstack-protector-all")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U_FORTIFY_SOURCE -fstack-protector-all")
endif ()


target_link_libraries(videodec PRIVATE ${LINNK_LIBRARYS})
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-deprecated-declarations")

if (HAVE_COVERAGE_CONFIG)
    target_link_libraries(videodec PUBLIC coverage_config)
endif ()
