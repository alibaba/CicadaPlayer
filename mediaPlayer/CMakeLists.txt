cmake_minimum_required(VERSION 3.6)
project(media_player)

if (NOT "${ANDROID_PLATFORM}" STREQUAL "")
    set(TARGET_PLATFORM Android)
endif ()

if (IOS)
    message("build ios")
    set(TARGET_PLATFORM iOS)
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES Windows)
    message("build windows")
    set(TARGET_PLATFORM windows)
endif ()


if (TARGET_PLATFORM)
else ()
    set(TARGET_PLATFORM macOSX)
endif ()

message("TARGET_PLATFORM is ${TARGET_PLATFORM} ")

if ("${TARGET_PLATFORM}" STREQUAL "Android")
    message("build Android")
endif ()

set(TOPDIR ${PROJECT_SOURCE_DIR}/..)

add_subdirectory(${TOPDIR}/framework framework.out)

include(${TOPDIR}/framework/${TARGET_PLATFORM}.cmake)

set(CMAKE_CXX_STANDARD 11)

message("COMMON_INC_DIR is ${COMMON_INC_DIR}")

if (ENABLE_CACHE_MODULE)
    add_definitions(-DENABLE_CACHE_MODULE)
endif ()

if (ENABLE_MUXER)
    add_definitions(-DENABLE_MUXER)
endif ()

set(SOURCE_FILES
        analytics/IAnalyticsCollector.h
        analytics/IAnalyticsPlayer.h
        analytics/IAnalyticsRender.h
        analytics/IAnalyticsSource.h
        analytics/AnalyticsType.h
        analytics/AnalyticsCollectorFactory.h
        analytics/AnalyticsCollectorImpl.h
        analytics/AnalyticsCollectorListener.h
        analytics/AnalyticsQueryListener.h
        analytics/AnalyticsUtils.h
        analytics/IAnalyticsCollector.cpp
        analytics/AnalyticsCollectorFactory.cpp
        analytics/AnalyticsCollectorImpl.cpp
        analytics/AnalyticsUtils.cpp
        MediaPlayerUtil.h
        MediaPlayerUtil.cpp
        MediaPlayer.h
        media_player_error_def.h
        media_player_api.h
        SuperMediaPlayer.h
        native_cicada_player_def.h
        media_player_api.cpp
        SuperMediaPlayer.cpp
        buffer_controller.cpp
        media_packet_queue.h
        media_packet_queue.cpp
        player_msg_control.cpp
        system_refer_clock.cpp
        player_types.cpp
        ICicadaPlayer.h
        player_types.cpp
        player_types.h
        player_notifier.cpp
        player_notifier.h
        native_cicada_player_def.h
        MediaPlayer.cpp
        MediaPlayerConfig.cpp
        QueryListener.cpp
        QueryListener.h
        abr/AbrManager.h
        abr/AbrManager.cpp
        abr/AbrRefererData.h
        abr/AbrRefererData.cpp
        abr/AbrBufferAlgoStrategy.h
        abr/AbrBufferAlgoStrategy.cpp
        abr/AbrAlgoStrategy.h
        abr/AbrAlgoStrategy.cpp
        abr/AbrBufferRefererData.cpp
        abr/AbrBufferRefererData.h
        SuperMediaPlayerDataSourceListener.cpp
        SuperMediaPlayerDataSourceListener.h
        PlayerCacheDataSource.cpp
        PlayerCacheDataSource.h
        EventCodeMap.cpp
        EventCodeMap.h
        ErrorCodeMap.cpp
        ErrorCodeMap.h
        subTitle/subTitlePlayer.cpp
        subTitle/subTitlePlayer.h
        subTitle/subTitleSource.cpp
        subTitle/subTitleSource.h
        mediaPlayerSubTitleListener.cpp
        mediaPlayerSubTitleListener.h
        playerOptions.cpp
        playerOptions.h
        SMP_DCAManager.cpp
        CicadaPlayerPrototype.cpp
        SMPAVDeviceManager.cpp
        SMPAVDeviceManager.h
        SMPRecorderSet.cpp
        SMPRecorderSet.h
        SMPMessageControllerListener.cpp
        SMPMessageControllerListener.h)

if(APPLE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fobjc-arc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc")
    set(SOURCE_FILES ${SOURCE_FILES}
            externalPlayer/AppleAVPlayer.mm
            externalPlayer/AppleAVPlayer.h
            externalPlayer/AppleAVPlayerLayerProcessor.mm
            externalPlayer/AppleAVPlayerLayerProcessor.h
			externalPlayer/AppleAVPlayerHandler.h
			externalPlayer/AppleAVPlayerHandler.mm
			externalPlayer/AppleAVPlayerUtil.h
			externalPlayer/AppleAVPlayerUtil.m
            )
#elseif(ANDROID)
#    set(SOURCE_FILES ${SOURCE_FILES}
#            externalPlayer/JNIPlayer.cpp
#            externalPlayer/JNIPlayer.h
#            )
endif()
if (TARGET_PLATFORM STREQUAL "Android")
    set(SOURCE_FILES ${SOURCE_FILES}
            TrafficStats.c
            )
endif ()

#if (TARGET_PLATFORM STREQUAL "windows")
#    set(SOURCE_FILES ${SOURCE_FILES}
#            AlivcDxva2Render.cpp
#            )
#endif ()

if (TARGET_PLATFORM STREQUAL "windows")
else ()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-inconsistent-missing-override")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -fstack-protector-all")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2 -fstack-protector-all")
endif ()

if ($ENV{VERSION})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVERSION=$ENV{VERSION}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVERSION=$ENV{VERSION}")
endif ()

add_library(media_player STATIC ${SOURCE_FILES})
target_include_directories(media_player PUBLIC
        ../framework
        ${COMMON_INC_DIR}
        ./)
if (HAVE_COVERAGE_CONFIG)
    target_link_libraries(media_player PUBLIC coverage_config)
endif ()

