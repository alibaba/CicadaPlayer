cmake_minimum_required(VERSION 3.6)

set(TOPDIR ${PROJECT_SOURCE_DIR}/../../../..)
include(android.cmake)

set(CMAKE_CXX_STANDARD 11)

set(TARGET_PLATFORM Android)

add_definitions(-D__STDC_FORMAT_MACROS)
add_subdirectory(../../../../plugin plugin.out)

include_directories(
        ${COMMON_INC_DIR}
        ${TOPDIR}/mediaPlayer/
        ${TOPDIR}/mediaPlayer/external/install/cares/Android/${ANDROID_ABI}/include
)

link_directories(
        ${BOOST_LIB_DIR}
        ${COMMON_LIB_DIR}
)

#set(CMAKE_CXX_VISIBILITY_PRESET hidden)
#set(CMAKE_C_VISIBILITY_PRESET hidden)
#set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

add_subdirectory(${TOPDIR}/mediaPlayer/ mediaPlayer.out)

find_library(log-lib log)

add_library(CicadaPlayer SHARED
        src/main/jni/JniLoader.cpp
        src/main/jni/player/JavaPlayerConfig.cpp
        src/main/jni/player/JavaCacheConfig.cpp
        src/main/jni/player/NativeBase.cpp
        src/main/jni/player/JavaTrackInfo.cpp
        src/main/jni/player/JavaMediaInfo.cpp
        src/main/jni/player/JavaGlobalSettings.cpp
        src/main/jni/utils/JavaLogger.cpp

        )

target_link_libraries(CicadaPlayer
        media_player
        cacheModule
        muxer
        demuxer
        plugin
        data_source
        render
        videodec
        framework_filter
        framework_utils
        alivcffmpeg
        GLESv2
        EGL
        OpenSLES
        android
        z
        ${log-lib}
        )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-inconsistent-missing-override")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -U_FORTIFY_SOURCE -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -U_FORTIFY_SOURCE -fstack-protector-all")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=return-type")

if (USEASAN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer -fsanitize-address-use-after-scope")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -fsanitize-address-use-after-scope")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
    #    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fsanitize=address")
endif (USEASAN)

