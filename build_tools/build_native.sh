#!/usr/bin/env bash


BUILD_TOOLS_DIR=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)

PATH=$PATH:${BUILD_TOOLS_DIR}

source env.sh
source common_build.sh
source utils.sh

CWD=$PWD

function create_cmake_config(){
   echo "cmake_minimum_required(VERSION 3.6)" >> $CONFIG_FILE
   echo "set(LIB_NAME ${LIB_NAME})" >> $CONFIG_FILE
   echo "find_library(AUDIO_TOOL_BOX AudioToolbox)" >> $CONFIG_FILE
   echo "find_library(VIDEO_TOOL_BOX VideoToolbox)" >> $CONFIG_FILE
   echo "find_library(COREMEDIA CoreMedia)" >> $CONFIG_FILE
   echo "find_library(COREVIDEO CoreVideo)" >> $CONFIG_FILE
   echo "find_library(COREFOUNDATION CoreFoundation)" >> $CONFIG_FILE
   echo "find_library(SECURITY Security)" >> $CONFIG_FILE
   echo -n "set(SRC_LIBRARIES ${SRC_LIBRARIES}" >> $CONFIG_FILE
   echo -n ' ${AUDIO_TOOL_BOX}' >> $CONFIG_FILE
   echo -n ' ${VIDEO_TOOL_BOX}' >> $CONFIG_FILE
   echo -n ' ${COREMEDIA}' >> $CONFIG_FILE
   echo -n ' ${COREVIDEO}' >> $CONFIG_FILE
   echo -n ' ${COREFOUNDATION}' >> $CONFIG_FILE
   if [[ "${SSL_USE_NATIVE}" == "TRUE" ]];then
      echo -n ' ${SECURITY}' >> $CONFIG_FILE
   fi
   echo ")" >> $CONFIG_FILE
   echo "set(SRC_LIBRARIES_DIR ${SRC_LIBRARIES_DIR})" >>$CONFIG_FILE
}

#build to ffmpeg
function build_shared_framework(){
    if [ -z "${LIB_NAME}" ];then
        export LIB_NAME=ffmpeg
    fi
    SRC_LIBRARIES="$(cd ./install/ffmpeg/Darwin/x86_64/lib; ls *.a)"
    SRC_LIBRARIES="$SRC_LIBRARIES z iconv resolv bz2"
    export CC=
    export AS=
    export CPU_FLAGS=
    export LDFLAGS=

    local support_libs="fdk-aac x264 curl openssl librtmp cares dav1d"

    SRC_LIBRARIES_DIR="$CWD/install/ffmpeg/Darwin/x86_64/lib"

    for support_lib in ${support_libs}
    do
        if [ -d "install/${support_lib}/Darwin/x86_64/lib" ];then
            SRC_LIBRARIES_DIR="$SRC_LIBRARIES_DIR $CWD/install/${support_lib}/Darwin/x86_64/lib"
            local libs="$(cd install/${support_lib}/Darwin/x86_64/lib; ls *.a)"
            SRC_LIBRARIES="$SRC_LIBRARIES $libs"
        fi
    done

    cd ./install/ffmpeg/Darwin/
    cp ${BUILD_TOOLS_DIR}/iOS/CMakeLists.txt ./

    CONFIG_FILE=config.cmake
    rm -rf ${CONFIG_FILE}

    create_cmake_config

    cp ${BUILD_TOOLS_DIR}/src/build_version.cpp ./
    sh ${BUILD_TOOLS_DIR}/gen_build_version.sh > version.h
    rm -rf Xcode/
    mkdir -p Xcode
    cd Xcode
    cmake ../ -G Xcode
    xcodebuild -configuration MinSizeRel -target ALL_BUILD

    return;
}

build_libs $(uname) x86_64 #$((uname -m)) Travis use uname -m get a 0...

if [[ "$(uname)" == "Darwin"  &&  "${BUILD_SHARED_LIB}" != "FALSE" ]];then
    build_shared_framework
fi