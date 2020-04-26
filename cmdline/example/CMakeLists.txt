cmake_minimum_required(VERSION 3.15)
project(example)

set(CMAKE_CXX_STANDARD 11)

include(../../framework/${TARGET_PLATFORM}.cmake)

include(../../framework/module_config.cmake)

add_executable(syncPlayer "")

target_sources(syncPlayer PRIVATE
        syncPlayer.cpp
        communication/playerMessage.cpp
        ../cicadaEventListener.cpp
        ../NetWorkEventReceiver.cpp
        )

if (ENABLE_SDL)
    target_sources(syncPlayer PRIVATE
            ../SDLEventReceiver.cpp
            ../SDLEventReceiver.h
            )
endif ()

target_include_directories(syncPlayer PUBLIC
        ../../mediaPlayer
        ../)
target_link_directories(syncPlayer PRIVATE
        ../../external/install/ffmpeg/${CMAKE_SYSTEM_NAME}/x86_64/lib
        ../../external/install/curl/${CMAKE_SYSTEM_NAME}/x86_64/lib
        ../../external/install/openssl/${CMAKE_SYSTEM_NAME}/x86_64/lib
        ${COMMON_LIB_DIR})

target_link_libraries(syncPlayer PRIVATE
        media_player
        demuxer
        data_source
        render
        videodec
        communication)
if (ENABLE_CACHE_MODULE)
    target_link_libraries(syncPlayer PRIVATE
            cacheModule)
endif ()
if (ENABLE_MUXER)
    target_link_libraries(syncPlayer PRIVATE
            muxer)
endif ()
target_link_libraries(syncPlayer PRIVATE
        framework_filter
        framework_utils
        avfilter
        avformat
        avcodec
        swresample
        avutil
        curl
        ${FRAMEWORK_LIBS})
if (ENABLE_SDL)
    target_link_libraries(syncPlayer PUBLIC
            SDL2
            )
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(syncPlayer PUBLIC
            bcrypt
            )
else ()
    target_link_libraries(syncPlayer PUBLIC
            z
            dl
            )
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(syncPlayer PUBLIC
            bz2
            )
endif ()

if (APPLE)
    target_link_libraries(
            syncPlayer PUBLIC
            iconv
            bz2
            ${FRAMEWORK_LIBS}
    )
else ()
    target_link_libraries(
            syncPlayer PUBLIC
            dl
            ssl
            crypto
            pthread
    )
endif ()
target_link_libraries(syncPlayer PUBLIC coverage_config)

if (USEASAN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer -fsanitize-address-use-after-scope")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -fsanitize-address-use-after-scope")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
    #    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fsanitize=address")
endif (USEASAN)

if (USETSAN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=thread")
    #    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fsanitize=address")
endif (USETSAN)

if (USEUBSAN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined, -fsanitize=integer,  -fsanitize=nullability")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined, -fsanitize=integer,  -fsanitize=nullability")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=undefined, -fsanitize=integer, -fsanitize=nullability")
    #    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fsanitize=address")
endif (USEUBSAN)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    if (USEMSAN)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=memory, -fsanitize-memory-track-origins -fno-omit-frame-pointer")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory, -fsanitize-memory-track-origins -fno-omit-frame-pointer")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=memory, -fsanitize-memory-track-origins -fno-omit-frame-pointer")
        #    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fsanitize=address")
    endif (USEMSAN)
endif ()