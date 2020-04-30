cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 11)
project(render)

list(APPEND SRC_FILES
        video/vsync/timedVSync.cpp
        video/vsync/timedVSync.h
        video/vsync/VSyncFactory.cpp
        video/vsync/VSyncFactory.h
        renderFactory.cpp
        renderFactory.h
        audio/filterAudioRender.cpp
        audio/filterAudioRender.h
        audio/audioRenderPrototype.cpp
        audio/audioRenderPrototype.h)

#if (APPLE)
#    list(APPEND SRC_FILES
#            video/vsync/CADisplayLinkVSync.mm
#            video/vsync/CADisplayLinkVSync.h
#            video/vsync/CAApplicationCaller.mm
#            video/vsync/CAApplicationCaller.h)
#endif ()

if (ANDROID)
    list(APPEND SRC_FILES
            video/vsync/AndroidVSync.cpp
            video/vsync/AndroidVSync.h
            )
endif ()

if (ENABLE_SDL)
    list(APPEND SRC_FILES
            audio/SdlAFAudioRender.h
            audio/SdlAFAudioRender.cpp
            video/SdlAFVideoRender.cpp
            video/SdlAFVideoRender.h
            )
endif ()

if (ENABLE_CHEAT_RENDER)
    list(APPEND SRC_FILES
            video/CheaterVideoRender.cpp
            audio/CheaterAudioRender.cpp
            )
endif ()

if (ANDROID)
    list(APPEND SRC_FILES
            audio/filterAudioRender.cpp
            audio/filterAudioRender.h
            audio/Android/AudioTrackRender.cpp
            audio/Android/AudioTrackRender.h
            )
endif ()


if (APPLE)
    list(APPEND SRC_FILES
            audio/Apple/AudioRenderType.h
            audio/Apple/AFAudioUnitRender.cpp
            audio/Apple/AFAudioUnitRender.h
            )
endif ()

if (IOS)
    list(APPEND SRC_FILES
            audio/Apple/AFAudioSession.mm
            audio/Apple/AFAudioSession.h
            audio/Apple/AFAudioSessionWrapper.mm
            audio/Apple/AFAudioSessionWrapper.h
            audio/Apple/CicadaAudioSessionDelegate.h
            )
endif ()

if (ENABLE_GLRENDER)
    add_definitions(-DGLRENDER)

    file(GLOB FilesPlatformBase video/glRender/platform/*.*)
    # TODO
    if (ANDROID)
        file(GLOB FilesPlatform video/glRender/platform/android/*.*)
    endif ()

    if (IOS)
        file(GLOB FilesPlatform video/glRender/platform/ios/*.*)
    endif ()

    #

    if (APPLE AND NOT IOS)
        file(GLOB FilesPlatform video/glRender/platform/mac/*.*)
    endif ()
    #
    #    if (${TARGET_PLATFORM} STREQUAL "windows")
    #        file(GLOB FilesPlatform      glRender/platform/window/*.*)
    #        ADD_DEFINITIONS(-DGLEW_STATIC)
    #    endif()

    file(GLOB FilesRenderSystem video/glRender/render_system/*.*)
    file(GLOB FilesEGL video/glRender/render_system/EGL/*.*)
    file(GLOB FilesBase video/glRender/base/*.*)

    list(APPEND SRC_FILES
            video/glRender/GLRender.cpp
            video/glRender/GLRender.h
            video/glRender/IProgramContext.h
            video/glRender/YUVProgramContext.cpp
            video/glRender/YUVProgramContext.h

            ${FilesPlatformBase}
            ${FilesPlatform}

            ${FilesRenderSystem}
            ${FilesEGL}

            ${FilesBase}

            )

    if (ANDROID)
        set(SRC_FILES ${SRC_FILES}
                video/glRender/OESProgramContext.cpp
                video/glRender/OESProgramContext.h
                video/glRender/platform/android/decoder_surface.cpp
                video/glRender/platform/android/decoder_surface.h
                )
    endif ()

    if (IOS)
        set(SRC_FILES ${SRC_FILES}
                video/glRender/CV420PProgramContext.cpp
                video/glRender/CV420PProgramContext.h)
    endif ()

endif ()

add_library(render STATIC ${SRC_FILES})

target_include_directories(render PUBLIC ${COMMON_INC_DIR})
if (ENABLE_SDL)
    target_compile_definitions(render PUBLIC ENABLE_SDL)
    target_include_directories(render PUBLIC /usr/local/include/)
    target_link_directories(render PRIVATE ${COMMON_LIB_DIR})
    target_link_directories(render PUBLIC /usr/local/lib)
    target_link_libraries(render PUBLIC SDL2)
endif ()

if (ENABLE_CHEAT_RENDER)
    target_compile_definitions(render PUBLIC ENABLE_CHEAT_RENDER)
endif ()

if (ENABLE_GLRENDER)

    target_include_directories(render PUBLIC
            video/glRender/
            )

endif ()


if (HAVE_COVERAGE_CONFIG)
    target_link_libraries(render PUBLIC coverage_config)
endif ()
