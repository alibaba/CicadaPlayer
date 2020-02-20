#!/usr/bin/env bash
BUILD_TOOLS_DIR=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)

PATH=$PATH:${BUILD_TOOLS_DIR}
source env.sh
source build_fdk_aac.sh
source ffmpeg_commands.sh
source build_ffmpeg.sh
source common_build.sh
source build_x264.sh
source build_openssl.sh
source build_ares.sh
source build_curl.sh
CWD=$PWD
function build_fat_lib(){

    local lib_names=$(cd ./install/$1/iOS/${IOS_ARCHS%% *}/lib; ls *.a)
    if  [ ! -n "$lib_names" ] ;then
        echo "break create $1 fat"
        return
    fi

    local fat_dir=install/$1/iOS/fat/lib
    rm -rf ${fat_dir}
    mkdir -p  ${fat_dir}
    for libname in ${lib_names}
    do
         libs=$(find install/$1/iOS -name ${libname})
         lipo -create ${libs}  -output install/$1/iOS/fat/lib/${libname}
    done
}

function build_fat_libs(){
    lib_names=$(cd ./install/; ls)
    for libname in ${lib_names}
    do
        build_fat_lib ${libname}
    done
}

function create_cmake_config(){
   echo "cmake_minimum_required(VERSION 3.6)" >> $CONFIG_FILE
   echo "set(LIB_NAME ${LIB_NAME})" >> $CONFIG_FILE
   echo "find_library(SECURITY Security)" >> $CONFIG_FILE
   echo "find_library(AUDIO_TOOL_BOX AudioToolbox)" >> $CONFIG_FILE
#   echo "find_library(VIDEO_TOOL_BOX VideoToolbox)" >> $CONFIG_FILE
#   echo "find_library(COREMEDIA CoreMedia)" >> $CONFIG_FILE
#   echo "find_library(COREVIDEO CoreVideo)" >> $CONFIG_FILE
#   echo "find_library(COREFOUNDATION CoreFoundation)" >> $CONFIG_FILE
   echo -n "set(SRC_LIBRARIES ${SRC_LIBRARIES}" >> $CONFIG_FILE
   if [[ "${SSL_USE_NATIVE}" == "TRUE" ]];then
       echo -n ' ${SECURITY}' >> $CONFIG_FILE
   fi
   echo -n ' ${AUDIO_TOOL_BOX}' >> $CONFIG_FILE
#   echo -n ' ${VIDEO_TOOL_BOX}' >> $CONFIG_FILE
#   echo -n ' ${COREMEDIA}' >> $CONFIG_FILE
#   echo -n ' ${COREVIDEO}' >> $CONFIG_FILE
#   echo -n ' ${COREFOUNDATION}' >> $CONFIG_FILE
   echo ")" >> $CONFIG_FILE
   echo "set(SRC_LIBRARIES_DIR ${SRC_LIBRARIES_DIR})" >>$CONFIG_FILE
}
#build to ffmpeg
function build_shared_framework(){
    if [ -z "${LIB_NAME}" ];then
        export LIB_NAME=ffmpeg
    fi
    SRC_LIBRARIES="$(cd ./install/ffmpeg/iOS/fat/lib; ls)"
    SRC_LIBRARIES="$SRC_LIBRARIES z iconv resolv bz2"
    export CC=
    export AS=
    export CPU_FLAGS=
    export LDFLAGS=

    local support_libs="fdk-aac x264 curl openssl librtmp cares dav1d"

    SRC_LIBRARIES_DIR="$CWD/install/ffmpeg/iOS/fat/lib"

    for support_lib in ${support_libs}
    do
        if [ -d "install/${support_lib}/iOS/fat/lib" ];then
            SRC_LIBRARIES_DIR="$SRC_LIBRARIES_DIR $CWD/install/${support_lib}/iOS/fat/lib"
            local libs="$(cd install/${support_lib}/iOS/fat/lib; ls)"
            SRC_LIBRARIES="$SRC_LIBRARIES $libs"
        fi
    done

    if [ -d "${DAV1D_EXTERNAL_DIR}/iOS/fat" ];then
        SRC_LIBRARIES_DIR="$SRC_LIBRARIES_DIR ${DAV1D_EXTERNAL_DIR}/iOS/fat/lib"
        SRC_LIBRARIES="$SRC_LIBRARIES dav1d"
    fi

 #   $CWD/install/openssl/iOS/fat/lib"
    cd ./install/ffmpeg/iOS/
    cp ${BUILD_TOOLS_DIR}/iOS/CMakeLists.txt ./

    CONFIG_FILE=config.cmake
    rm -rf ${CONFIG_FILE}

    create_cmake_config

    touch dummy.c
    rm -rf Xcode/
    mkdir -p Xcode/OS
    cd Xcode/OS
    cmake  -DCMAKE_TOOLCHAIN_FILE=${BUILD_TOOLS_DIR}/iOS/iOS.cmake \
    -DIOS_PLATFORM=OS ../../  \
    -G Xcode
    xcodebuild -destination generic/platform=iOS -configuration Release -target ALL_BUILD

    cd -
    mkdir -p Xcode/SIMULATOR64
    cd Xcode/SIMULATOR64
    cmake  -DCMAKE_TOOLCHAIN_FILE=${BUILD_TOOLS_DIR}/iOS/iOS.cmake \
    -DIOS_PLATFORM=SIMULATOR64 ../../  \
    -G Xcode

    xcodebuild -destination "platform=iOS Simulator" -configuration Release -target ALL_BUILD

    return;
}

iOS_check_env.sh

build_libs iOS "${IOS_ARCHS}"
build_fat_libs
build_shared_framework
