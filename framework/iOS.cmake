message("IOS_PLATFORM is ${IOS_PLATFORM}")
if (${IOS_PLATFORM})
    message("build ios ${IOS_PLATFORM}")
else ()
    set(IOS_PLATFORM OS)
endif ()

set(IPHONEOS_DEPLOYMENT_TARGET 8.0)

set(CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET ${IPHONEOS_DEPLOYMENT_TARGET})

set(IOS_INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR}/../external/install)
set(CICADA_TOP ${CMAKE_CURRENT_LIST_DIR}/..)

set(COMMON_INC_DIR ${COMMON_INC_DIR}
        ${IOS_INSTALL_DIR}/ffmpeg/iOS/arm64/include
        ${IOS_INSTALL_DIR}/../build/ffmpeg/iOS/arm64/
        ${CICADA_TOP}/external/boost/
        ${CICADA_TOP}/external/external/ffmpeg/
        ${IOS_INSTALL_DIR}/curl/iOS/arm64/include
        ${IOS_INSTALL_DIR}/openssl/iOS/arm64/include
        ${PROJECT_SOURCE_DIR}
        )

set(FFMPEG_SOURCE_DIR ${CICADA_TOP}/external/external/ffmpeg)

#find_library(COCOA Cocoa)
find_library(VIDEO_TOOL_BOX VideoToolbox)
find_library(AUDIO_TOOL_BOX AudioToolbox)
find_library(COREMEDIA CoreMedia)
find_library(COREVIDEO CoreVideo)
find_library(COREFOUNDATION CoreFoundation)
#find_library(VIDEODECODERACCELERATION VideoDecodeAcceleration)
find_library(COREFOUNDATION CoreFoundation)
find_library(SECURITY Security)

set(FRAMEWORK_LIBS
        #${COCOA}
        ${VIDEO_TOOL_BOX}
        ${AUDIO_TOOL_BOX}
        ${COREMEDIA}
        ${COREVIDEO}
        #        ${VIDEODECODERACCELERATION}
        ${COREFOUNDATION}
        ${SECURITY}
        ${COREFOUNDATION})

set(TARGET_LIBRARY_TYPE STATIC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fembed-bitcode -mios-version-min=${IPHONEOS_DEPLOYMENT_TARGET}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fembed-bitcode -mios-version-min=${IPHONEOS_DEPLOYMENT_TARGET}")
message("CMAKE_CXX_FLAGS is ${CMAKE_CXX_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -Os -pipe -fPIC -Wall -fmessage-length=0 -fvisibility=hidden")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -Os -pipe -fPIC -Wall -fmessage-length=0 -fvisibility=hidden")

set(BUILD_TEST OFF)
set(ENABLE_GLRENDER ON)
