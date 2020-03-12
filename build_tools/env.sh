#!/bin/bash

#set source dirs and ndk path

source utils.sh

FFMPEG_SOURCE_DIR=
FDK_AAC_SOURCE_DIR=
X264_SOURCE_DIR=
OPEN_SSL_SOURCE_DIR=
CURL_SOURCE_DIR=
DAV1D_SOURCE_DIR=
ARES_SOURCE_DIR=
RTMPDUMP_SOURCE_DIR=
IOS_ARCHS="armv7 arm64 x86_64 i386"
ANDROID_ABIS="armeabi-v7a arm64-v8a"
WIN32_ARCHS="i686 x86_64"

OPENSSL_VERSION_111="False"

BUILD=
#BUILD="False"
BUILD_FFMPEG=
#BUILD_FFMPEG="False"

if [[ -f $PWD/user_env.sh ]];then
    source user_env.sh
else
    print_error "user_env.sh not found"
    exit 1
fi
