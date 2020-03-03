

set(INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR}/../external/install)

set(COMMON_LIB_DIR ${COMMON_LIB_DIR}
        ${INSTALL_DIR}/curl/Linux/x86_64/lib
        ${INSTALL_DIR}/librtmp/Linux/x86_64/lib
        ${INSTALL_DIR}/openssl/Linux/x86_64/lib
        ${INSTALL_DIR}/ffmpeg/Linux/x86_64/lib
        ${INSTALL_DIR}/fdk-aac/Linux/x86_64/lib
        ${INSTALL_DIR}/cares/Linux/x86_64/lib
        )
set(COMMON_INC_DIR ${COMMON_INC_DIR}
        ${INSTALL_DIR}/curl/Linux/x86_64/include
        ${INSTALL_DIR}/librtmp/Linux/x86_64/include
        ${INSTALL_DIR}/openssl/Linux/x86_64/include
        ${INSTALL_DIR}/ffmpeg/Linux/x86_64/include
        ${INSTALL_DIR}/cares/Linux/x86_64/include
        ${INSTALL_DIR}/../build/ffmpeg/Linux/x86_64/
        ${INSTALL_DIR}/../boost/
        ${INSTALL_DIR}/../external/external/ffmpeg/
        ${PROJECT_SOURCE_DIR})


set(FFMPEG_SOURCE_DIR ${TOPDIR}/external/external/ffmpeg/)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=return-type")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror=return-type")

#find_library(VIDEO_TOOL_BOX VideoToolbox)
#find_library(AUDIO_TOOL_BOX AudioToolbox)
#find_library(COREMEDIA CoreMedia)
#find_library(COREVIDEO CoreVideo)
#find_library(COREFOUNDATION CoreFoundation)
#find_library(VIDEODECODERACCELERATION VideoDecodeAcceleration)
#find_library(COREFOUNDATION CoreFoundation)
#find_library(SECURITY Security)
#find_library(OPENGL OpenGL)
#find_library(APPKIT AppKit)
#
#set(FRAMEWORK_LIBS
#        ${VIDEO_TOOL_BOX}
#        ${AUDIO_TOOL_BOX}
#        ${COREMEDIA}
#        ${COREVIDEO}
#        ${VIDEODECODERACCELERATION}
#        ${COREFOUNDATION}
#        ${SECURITY}
#        ${COREFOUNDATION}
#        ${OPENGL}
#        ${APPKIT}
#        iconv
#        z)


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

if (USEMSAN)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=memory, -fsanitize-memory-track-origins -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=memory, -fsanitize-memory-track-origins -fno-omit-frame-pointer")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fsanitize=memory, -fsanitize-memory-track-origins -fno-omit-frame-pointer")
    #    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} -fsanitize=address")
endif (USEMSAN)

set(TARGET_LIBRARY_TYPE STATIC)

set(ENABLE_GLRENDER OFF)

set(BUILD_TEST ON)
if (TRAVIS)
    set(ENABLE_CHEAT_RENDER ON)
else ()
    set(ENABLE_SDL ON)
endif ()

