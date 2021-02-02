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
find_library(AV_FOUNDATION AVFoundation)


message("xxxxxxxxxxxxxxxxxxxxxxxx TOP_DIR is ${TOP_DIR}")

if ("${TARGET_PLATFORM}" STREQUAL "iOS")
    find_library(ALIVCFFMPEG alivcffmpeg
            ${TOP_DIR}/apsaraPlayer/external/install/ffmpeg/iOS/Xcode/OS/_builds/ NO_DEFAULT_PATH)
    set(ALIVCFFMPEG ${TOP_DIR}/apsaraPlayer/external/install/ffmpeg/iOS/Xcode/OS/_builds/alivcffmpeg.framework)
    message("CONAN is ${CONAN}")
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
        framework_drm
        cacheModule
        videodec
        muxer
        render
        communication
        z
        iconv
        resolv
        bz2
        c++
        )

set(ALI_SRC_LIBRARIES ${ALI_SRC_LIBRARIES} "-framework CoreMedia -framework CoreVideo -framework VideoToolbox -framework CoreFoundation -framework CoreGraphics -framework AudioToolbox -framework CoreAudio -framework OpenAl -framework CoreImage -framework AVFoundation -framework QuartzCore")
if (IOS)
    set(ALI_SRC_LIBRARIES ${ALI_SRC_LIBRARIES} "-framework UIKit -framework OpenGLES")
else ()
    set(ALI_SRC_LIBRARIES ${ALI_SRC_LIBRARIES} "-framework AppKit -framework OpenGl")
endif ()
