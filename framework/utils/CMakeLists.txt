cmake_minimum_required(VERSION 3.6)
project(framework_utils)
set(CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES
        AutoAVFrame.h
        CicadaJSON.h
        afThread.h
        frame_work_log.h
        mediaFrame.h
        timer.h
        af_string.h
        af_clock.h
        ringBuffer.h
        cJSON.h
        CicadaJSON.cpp
        afThread.cpp
        frame_work_log.c
        mediaFrame.c
        timer.cpp
        af_string.cpp
        errors/framework_error.c
        af_clock.cpp
        ringBuffer.cpp
        cJSON.c
        globalSettings.cpp
        property.cpp
        err.cpp
        err.h
        CicadaType.h
        bitStreamParser.cpp
        CicadaUtils.h
        CicadaUtils.cpp
        AFUtils.c
        AsyncJob.cpp
        AsyncJob.h
        CicadaDynamicLoader.h
        CicadaDynamicLoader.cpp
        file/FileCntl.cpp
        file/FileCntl.h
        file/FileUtils.cpp
        file/FileUtils.h
        oscl/oscl_utils.cpp
        CicadaThumbnailParser.cpp
        CicadaThumbnailParser.h
        mediaTypeInternal.cpp
        mediaTypeInternal.h
        ffmpeg_utils.h
        ffmpeg_utils.c
        ../base/media/subTitlePacket.cpp
        ../base/media/subTitlePacket.h
        stringUtil.h
        Md5Utils.cpp
        Md5Utils.h
        ../base/options.cpp
        ../base/options.h
        )

list(APPEND SOURCE_FILES
        ../base/media/IAFPacket.cpp
        ../base/media/IAFPacket.h
        ../base/media/AVAFPacket.cpp
        ../base/media/AVAFPacket.h
        )

if (APPLE)
    list(APPEND SOURCE_FILES
            ../base/media/PBAFFrame.cpp
            ../base/media/PBAFFrame.h
            )
endif ()

if (MSVC)
    set(SOURCE_FILES ${SOURCE_FILES}
            ffmpegshim.c
            strtok_r.c
            mkstemp.c
            basename.c)
endif ()

if (ANDROID)
    set(SOURCE_FILES ${SOURCE_FILES}
            ../base/media/AFMediaCodecFrame.cpp
            ../base/media/AFMediaCodecFrame.h
            Android/cpu-features.c
            Android/AndroidJniHandle.cpp
            Android/AndroidJniHandle.h
            oscl/linux_utils.c
            Android/systemUtils.cpp
            Android/systemUtils.h
            Android/JniEnv.cpp
            Android/JniEnv.h
            Android/JniException.cpp
            Android/JniException.h
            Android/FindClass.cpp
            Android/FindClass.h
            Android/NewStringUTF.cpp
            Android/NewStringUTF.h
            Android/GetObjectClass.cpp
            Android/GetObjectClass.h
            Android/CallObjectMethod.cpp
            Android/CallObjectMethod.h
            Android/GetStringUTFChars.cpp
            Android/GetStringUTFChars.h
            Android/CallBooleanMethod.cpp
            Android/CallBooleanMethod.h
            Android/CallIntMethod.cpp
            Android/CallIntMethod.h
            Android/GetObjectField.cpp
            Android/GetObjectField.h
            Android/Convertor.cpp
            Android/Convertor.h
            )
elseif (IOS)
    set(SOURCE_FILES ${SOURCE_FILES}
            oscl/ios_utils.c
            ../base/media/subTitlePacket.cpp ../base/media/subTitlePacket.h)
endif ()

include_directories(
        ${COMMON_INC_DIR} ${FFMPEG_SOURCE_DIR})

add_library(framework_utils ${TARGET_LIBRARY_TYPE} ${SOURCE_FILES})
