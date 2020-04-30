cmake_minimum_required(VERSION 3.6)
find_library(VIDEO_TOOL_BOX VideoToolbox)
find_library(AUDIO_TOOL_BOX AudioToolbox)
find_library(CORE_AUDIO CoreAudio)
find_library(COREMEDIA CoreMedia)
find_library(COREVIDEO CoreVideo)
find_library(COREFOUNDATION CoreFoundation)
find_library(COREGRAPHICS CoreGraphics)
find_library(OPEN_AL OpenAl)
find_library(CORE_IMAGE CoreImage)
find_library(QUARTZ_CORE QuartzCore)

message("xxxxxxxxxxxxxxxxxxxxxxxx TOP_DIR is ${TOP_DIR}")

if ("${TARGET_PLATFORM}" STREQUAL "iOS")
    find_library(ALIVCFFMPEG alivcffmpeg
            ${TOP_DIR}/apsaraPlayer/external/install/ffmpeg/iOS/Xcode/OS/_builds/ NO_DEFAULT_PATH)
    set(ALIVCFFMPEG ${TOP_DIR}/apsaraPlayer/external/install/ffmpeg/iOS/Xcode/OS/_builds/alivcffmpeg.framework)
    message("CONAN is ${CONAN}")
    find_library(AV_FOUNDATION AVFoundation)
    find_library(UIKIT UIKit)
    find_library(OPENGLES OpenGLES)
else ()
    message("CONAN is ${CONAN}")
    find_library(APP_KIT AppKit)
    find_library(OPEN_GL OpenGl)
endif ()
message("VIDEO_TOOL_BOX is ${VIDEO_TOOL_BOX}")


set(ALI_SRC_LIBRARIES
        media_player
        demuxer
        data_source
        framework_filter
        framework_utils
        cacheModule
        videodec
        muxer
        render
        communication
        z
        iconv
        resolv
        bz2
        ${VIDEO_TOOL_BOX}
        ${COREMEDIA}
        ${COREVIDEO}
        ${COREFOUNDATION}
        ${COREGRAPHICS}
        ${AUDIO_TOOL_BOX}
        ${CORE_AUDIO}
        ${OPEN_AL}
        ${CORE_IMAGE}
        c++
        )
if (IOS)
    set(ALI_SRC_LIBRARIES ${ALI_SRC_LIBRARIES}
            ${AV_FOUNDATION}
            ${UIKIT}
            ${QUARTZ_CORE}
            ${OPENGLES}
            )
else ()
    set(ALI_SRC_LIBRARIES ${ALI_SRC_LIBRARIES}
#            ${CONAN}
            ${APP_KIT}
            ${OPEN_GL}
            )
endif ()
