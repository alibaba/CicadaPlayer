#!/bin/bash

#set source dirs and ndk path
echo TOP_DIR is $TOP_DIR
FFMPEG_SOURCE_DIR=$TOP_DIR/external/ffmpeg
FDK_AAC_SOURCE_DIR=$TOP_DIR/external/fdk-aac/
X264_SOURCE_DIR=$TOP_DIR/external/x264/
OPEN_SSL_SOURCE_DIR=$TOP_DIR/external/openssl/
CURL_SOURCE_DIR=$TOP_DIR/external/curl
#DAV1D_SOURCE_DIR=$TOP_DIR/external/dav1d
#ARES_SOURCE_DIR=$TOP_DIR/external/cares
#RTMPDUMP_SOURCE_DIR=$TOP_DIR/external/rtmpdump/
#ANDROID_NDK=$ANDROID_NDK_HOME
export LIB_NAME=alivcffmpeg
OPENSSL_VERSION_111="True"
SSL_USE_NATIVE="TRUE"

