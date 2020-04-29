

set(MAC_INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR}/../external/install)

set(COMMON_LIB_DIR ${COMMON_LIB_DIR}
        ${MAC_INSTALL_DIR}/curl/Darwin/x86_64/lib
        ${MAC_INSTALL_DIR}/librtmp/Darwin/x86_64/lib
        ${MAC_INSTALL_DIR}/openssl/Darwin/x86_64/lib
        ${MAC_INSTALL_DIR}/ffmpeg/Darwin/x86_64/lib
        ${MAC_INSTALL_DIR}/fdk-aac/Darwin/x86_64/lib
        ${MAC_INSTALL_DIR}/cares/Darwin/x86_64/lib
        ${MAC_INSTALL_DIR}/dav1d/Darwin/x86_64/lib
        )
set(COMMON_INC_DIR ${COMMON_INC_DIR}
        ${MAC_INSTALL_DIR}/curl/Darwin/x86_64/include
        ${MAC_INSTALL_DIR}/librtmp/Darwin/x86_64/include
        ${MAC_INSTALL_DIR}/openssl/Darwin/x86_64/include
        ${MAC_INSTALL_DIR}/ffmpeg/Darwin/x86_64/include
        ${MAC_INSTALL_DIR}/cares/Darwin/x86_64/include
        ${MAC_INSTALL_DIR}/../build/ffmpeg/Darwin/x86_64/
        ${MAC_INSTALL_DIR}/../boost/
        ${MAC_INSTALL_DIR}/../external/ffmpeg/
        ${PROJECT_SOURCE_DIR})


set(FFMPEG_SOURCE_DIR ${TOPDIR}/external/external/ffmpeg/)

find_library(VIDEO_TOOL_BOX VideoToolbox)
find_library(AUDIO_TOOL_BOX AudioToolbox)
find_library(COREMEDIA CoreMedia)
find_library(COREVIDEO CoreVideo)
find_library(COREFOUNDATION CoreFoundation)
find_library(VIDEODECODERACCELERATION VideoDecodeAcceleration)
find_library(COREFOUNDATION CoreFoundation)
find_library(SECURITY Security)
find_library(OPENGL OpenGL)
find_library(APPKIT AppKit)

set(FRAMEWORK_LIBS
        ${VIDEO_TOOL_BOX}
        ${AUDIO_TOOL_BOX}
        ${COREMEDIA}
        ${COREVIDEO}
        ${VIDEODECODERACCELERATION}
        ${COREFOUNDATION}
        ${SECURITY}
        ${COREFOUNDATION}
        ${OPENGL}
        ${APPKIT}
        iconv
        z)

set(TARGET_LIBRARY_TYPE STATIC)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=return-type")
if (USEASAN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer -fsanitize-address-use-after-scope")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer -fsanitize-address-use-after-scope")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=address")
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

if (TRAVIS)
    set(ENABLE_CHEAT_RENDER ON)
    set(ENABLE_SDL OFF)
else ()
    set(ENABLE_SDL ON)
endif ()

if (CMDLINE_BUILD)
    message("CMDLINE_BUILD")
    set(BUILD_TEST ON)
    set(ENABLE_SDL ON)
    set(ENABLE_GLRENDER OFF)
else ()
    set(ENABLE_GLRENDER ON)
    set(ENABLE_SDL OFF)
endif ()

