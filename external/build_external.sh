#!/usr/bin/env bash

BUILD_TOOLS_DIR=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)
export TOP_DIR=${PWD}
PATH=$PATH:${BUILD_TOOLS_DIR}
source ../build_tools/utils.sh
source user_env.sh

function patch_ffmpeg() {

    cd ${FFMPEG_SOURCE_DIR}
    git am ../../contribute/ffmpeg/0001-chore-win32-fix-not-find-openssl-1.1.1.patch

    if [ $? -ne 0 ]; then
        print_warning "patch error, may be patched"
        git am --abort
        return 0;
    fi
    git am ../../contribute/ffmpeg/0002-chore-avformat-change-MAX_PES_PAYLOAD-to-5M.patch
    git am ../../contribute/ffmpeg/0003-chore-libavformat-exoport-some-functions.patch
}

function load_source() {
    local user_sources=$(cd ${PWD};ls ../*_git_source_list.sh)
    local user_source
    echo "user_sources is ${user_sources}"
    if [ "${user_sources}" == "" ];then
        print_error "no source list config found"
        return 1;
    else
        for user_source in ${user_sources}
        do
            print_warning "apply "${user_source}""
            source ${user_source}
        done
    fi

    if [[ ${FFMPEG_NEED_PATCH} == "TRUE" ]];then
        patch_ffmpeg
    fi
    return 0;
}

function check_android_tools() {
    if [[ `which aarch64-linux-android-gcc` ]];then
        return 0;
    fi

    UNAME=$(uname)
    if [[ "$UNAME" = "Darwin" ]]
    then
        echo Darwin
        export HOST=darwin
    else
        echo Linux
        export HOST=linux
    fi

    if  [ -z "${ANDROID_NDK}" ];then
        ANDROID_NDK=$ANDROID_NDK_HOME #mtl's Android ndk
    else
        ANDROID_NDK_HOME=$ANDROID_NDK
    fi
    echo ANDROID_NDK is ${ANDROID_NDK}
    PATH=$PATH:${ANDROID_NDK}:${ANDROID_NDK}/toolchains/arm-linux-androideabi-4.9/prebuilt/${HOST}-x86_64/bin
    PATH=$PATH:${ANDROID_NDK}/toolchains/aarch64-linux-android-4.9/prebuilt/${HOST}-x86_64/bin
}

function apply_config() {
    local configs="$1";
    local config;

    for config in $configs ; do
        cp config/${config}_ffmpeg_config.sh ./
        cp config/${config}_git_source_list.sh ./
    done
}

function check_cmake(){
    if [ ! `which cmake` ]
    then
        echo 'cmake not found'
        if [ ! `which brew` ]
        then
            echo 'Homebrew not found. Trying to install...'
            ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" \
            || exit 1
        fi
        echo 'Trying to install cmake...'
        brew install cmake || exit 1
    fi
#    brew upgrade cmake
}
function check_yasm(){
    if [ ! `which yasm` ]
    then
        echo 'yasm not found'
        if [ ! `which brew` ]
        then
            echo 'Homebrew not found. Trying to install...'
            ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" \
            || exit 1
        fi
        echo 'Trying to install yasm...'
        brew install yasm
    fi
    echo `yasm --version`

    if [ ! `which nasm` ] || [ `which nasm` == "/usr/bin/nasm" ]
    then
        echo 'nasm not found'
        brew install nasm
        alias nasm=/usr/local/bin/nasm
        PATH=/usr/local/bin:$PATH
    else
        echo nasm is `which nasm`
    fi
    echo nasm is `which nasm`
    echo `nasm --version`
}


mkdir external
cd external
load_source
if [ $? -ne 0 ]; then
    echo "load_source error break"
    exit 1;
fi

cd ${TOP_DIR}

export TARGET_PLATFORM=$1

if [ "$1" == "Android" ];then
    if  [ -z "${ANDROID_NDK}" ];then
        export ANDROID_NDK=~/Android-env/android-ndk-r14b/
    fi
    check_android_tools
    ../build_tools/build_Android.sh

elif [ "$1" == "iOS" ];then
    #export HOMEBREW_NO_AUTO_UPDATE=true
    check_cmake
    check_yasm
    ../build_tools/build_iOS.sh
elif [ "$1" == "macOS" ];then
    ../build_tools/build_native.sh 
elif [ "$1" == "Linux" ];then
    ../build_tools/build_native.sh
elif [ "$1" == "Windows" ];then
    ../build_tools/build_win32.sh
fi




