cmake_minimum_required(VERSION 3.6)

option(CMDLINE_BUILD "build command line example" OFF)
project(CicadaPlayerSDK)

set(TOP_DIR "${CMAKE_SOURCE_DIR}/../../..")


if (IOS)
    set(TARGET_PLATFORM iOS)
    set(PATH_NAME iOS)
else ()
    set(TARGET_PLATFORM macOSX)
    set(PATH_NAME Darwin)
endif ()
include(config.cmake)

set(CMAKE_CXX_STANDARD 11)

set(ENV{DEPLOYMENT_TARGET} "8.0")

set(IPHONEOS_DEPLOYMENT_TARGET $ENV{DEPLOYMENT_TARGET})
set(CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET $ENV{DEPLOYMENT_TARGET})
set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fobjc-arc")
set(ALIBRARYS ${ALI_SRC_LIBRARIES})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
add_subdirectory(${TOP_DIR}/mediaPlayer mediaPlayer.out)
add_subdirectory(${TOP_DIR}/external/install/ffmpeg/${PATH_NAME} alivcffmpeg.out)
add_subdirectory(${TOP_DIR}/plugin plugin.out)

add_compile_options(-x objective-c++)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-abi-version=2 -fobjc-arc")

include_directories(
        ./thumbnail
        ${TOP_DIR}/mediaPlayer
        ${TOP_DIR}/framework
        ${TOP_DIR}/framework/render/video/glRender/platform/ios
        ${TOP_DIR}/framework/render/audio/Apple)

link_directories(${SRC_LIBRARIES_DIR})

set(SOURCE_LIST
        CicadaPlayer.mm
        CicadaPlayer.h
        CicadaSyncPlayerServer.h
        CicadaSyncPlayerServer.mm
        CicadaSyncPlayerClient.h
        CicadaSyncPlayerClient.mm
        playerMessage.h
        playerMessage.cpp
        CicadaPlayerGlobalSettings.h
        CicadaPlayerGlobalSettings.mm
        CicadaPlayerSDK.h
        CicadaConfig.mm
        CicadaConfig.h
        CicadaCacheConfig.mm
        CicadaCacheConfig.h
        CicadaDef.mm
        CicadaDef.h
        CicadaOCHelper.mm
        CicadaOCHelper.h
        CicadaMediaInfo.mm
        CicadaMediaInfo.h
        CicadaSource.mm
        CicadaSource.h
        CicadaDelegate.h
        CicadaErrorCode.h
        CicadaRenderDelegate.h
        CicadaRenderCBWrapper.h
        CicadaRenderCBWrapper.mm
        ${TOP_DIR}/framework/render/audio/Apple/CicadaAudioSessionDelegate.h
        thumbnail/CicadaThumbnail.h
        thumbnail/CicadaThumbnail.mm
        thumbnail/CicadaThumbnailItem.h
        thumbnail/CicadaThumbnailItem.mm
        thumbnail/CicadaURLSession.h
        thumbnail/CicadaURLSession.mm)

add_library(CicadaPlayerSDK SHARED
        ${SOURCE_LIST}
        )
message("LIBRARYS is ${ALIBRARYS}")
target_link_libraries(CicadaPlayerSDK PRIVATE ${ALIBRARYS})
target_link_libraries(CicadaPlayerSDK PUBLIC alivcffmpeg)

target_include_directories(CicadaPlayerSDK PUBLIC ${PROJECT_SOURCE_DIR})

set_source_files_properties(${XIB_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
set(PUBLIC_HEADERS
        CicadaPlayer.h
        CicadaSyncPlayerServer.h
        CicadaSyncPlayerClient.h
        CicadaPlayerGlobalSettings.h
        CicadaConfig.h
        CicadaCacheConfig.h
        CicadaDef.h
        CicadaMediaInfo.h
        CicadaSource.h
        CicadaPlayerSDK.h
        CicadaDelegate.h
        CicadaErrorCode.h
        CicadaRenderDelegate.h
        ${TOP_DIR}/framework/render/audio/Apple/CicadaAudioSessionDelegate.h
        )

if (IOS)
    set(BITCODE_VALUE "YES")
    set(BITCODE_GENERATION_VALUE bitcode)
else ()
    set(BITCODE_VALUE "NO")
    set(BITCODE_GENERATION_VALUE marker)
endif ()

set_target_properties(CicadaPlayerSDK PROPERTIES
        FRAMEWORK TRUE
        FRAMEWORK_VERSION A
        MACOSX_FRAMEWORK_IDENTIFIER com.alibaba.CicadaPlayerSDK
        LINK_FLAGS "-Wl,${LINK_OPTS}"
        PUBLIC_HEADER "${PUBLIC_HEADERS}"
        LINKER_LANGUAGE C
        MACOSX_FRAMEWORK_BUNDLE_VERSION 1
        MACOSX_FRAMEWORK_SHORT_VERSION_STRING 1.0
        INSTALL_NAME_DIR @rpath
        XCODE_ATTRIBUTE_LD_DYLIB_INSTALL_NAME "@rpath/CicadaPlayerSDK.framework/CicadaPlayerSDK"
        #        MACOSX_FRAMEWORK_INFO_PLIST Info.plist
        #XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
        XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS[variant=MinSizeRel] YES
        XCODE_ATTRIBUTE_ENABLE_BITCODE ${BITCODE_VALUE}
        XCODE_ATTRIBUTE_DEFINES_MODULE "YES"
        XCODE_ATTRIBUTE_BITCODE_GENERATION_MODE[variant=MinSizeRel] ${BITCODE_GENERATION_VALUE}
        )

set(INSTALL_TARGETS CicadaPlayerSDK)
#DON_T change the install lib's order
if (IOS)
    set(INSTALL_TARGETS ${INSTALL_TARGETS}
            alivcffmpeg
            CicadaPlayerSDK
            )

    install(TARGETS ${INSTALL_TARGETS}
            FRAMEWORK DESTINATION
            ${PROJECT_BINARY_DIR}/ARM_SIMULATOR)
else ()
    set(INSTALL_TARGETS ${INSTALL_TARGETS}
            alivcffmpeg
            CicadaPlayerSDK
            )

    install(TARGETS ${INSTALL_TARGETS}
            FRAMEWORK DESTINATION
            ${PROJECT_BINARY_DIR})
endif()
