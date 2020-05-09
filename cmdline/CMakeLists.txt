cmake_minimum_required(VERSION 3.15)
project(cicadaPlayer)

set(CMAKE_CXX_STANDARD 11)
option(CMDLINE_BUILD "build command line example" OFF)
set(CMDLINE_BUILD ON)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(TARGET_PLATFORM macOSX)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(TARGET_PLATFORM Linux)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(TARGET_PLATFORM windows)
endif ()

include(../framework/code_coverage.cmake)
add_subdirectory(../mediaPlayer mediaPlayer.out)

include(../framework/${TARGET_PLATFORM}.cmake)

include(../framework/module_config.cmake)

list(APPEND SRC_FILE
        cicadaPlayer.cpp
        NetWorkEventReceiver.cpp
        cicadaEventListener.cpp
        )

if (ENABLE_SDL)
    list(APPEND SRC_FILE
            SDLEventReceiver.cpp
            SDLEventReceiver.h
            )
endif ()

add_executable(cicadaPlayer
        ${SRC_FILE})
target_include_directories(cicadaPlayer PUBLIC ../mediaPlayer)
target_link_directories(cicadaPlayer PRIVATE
        ../external/install/ffmpeg/${CMAKE_SYSTEM_NAME}/x86_64/lib
        ../external/install/curl/${CMAKE_SYSTEM_NAME}/x86_64/lib
        ../external/install/openssl/${CMAKE_SYSTEM_NAME}/x86_64/lib
        ${COMMON_LIB_DIR})

target_link_libraries(cicadaPlayer PRIVATE
        media_player
        demuxer
        data_source
        render
        videodec)
if (ENABLE_CACHE_MODULE)
    target_link_libraries(cicadaPlayer PRIVATE
            cacheModule)
endif ()
if (ENABLE_MUXER)
    target_link_libraries(cicadaPlayer PRIVATE
            muxer)
endif ()
target_link_libraries(cicadaPlayer PRIVATE
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
    target_link_libraries(cicadaPlayer PUBLIC
            SDL2
            )
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    target_link_libraries(cicadaPlayer PUBLIC
            bcrypt
            )
else ()
    target_link_libraries(cicadaPlayer PUBLIC
            z
            dl
            )
endif ()

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(cicadaPlayer PUBLIC
            bz2
            )
endif ()

if (APPLE)
    target_link_libraries(
            cicadaPlayer PUBLIC
            iconv
            bz2
            ${FRAMEWORK_LIBS}
    )
else ()
    target_link_libraries(
            cicadaPlayer PUBLIC
            ssl
            crypto
            pthread
    )
endif ()
target_link_libraries(cicadaPlayer PUBLIC coverage_config)
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
else ()
    add_subdirectory(example)
endif ()

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
