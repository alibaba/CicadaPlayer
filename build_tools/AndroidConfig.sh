#!/usr/bin/env bash

function android_armv7_a_init_env(){
    NDK_V=14
    ABI=armeabi-v7a
    CPU_ARCH=arm
    CROSS_COMPILE=arm-linux-androideabi
    CPU_FLAGS="-march=armv7-a -mcpu=cortex-a8 -mfpu=vfpv3-d16 -mfloat-abi=softfp -mthumb"
    CPU_LD_FLAGS="-Wl,--fix-cortex-a8"
}

function android_arm64_v8a_init_env(){
    NDK_V=21
    ABI=arm64-v8a
    CPU_ARCH=arm64
    CROSS_COMPILE=aarch64-linux-android
    CPU_FLAGS=""
    CPU_LD_FLAGS=""
}
#CROSS_COMPILE=Android
function android_init_env(){
    if [ "$1" == "armeabi-v7a" ]
    then
        android_armv7_a_init_env
    elif [ "$1" == "arm64-v8a" ]
    then
        android_arm64_v8a_init_env
    else
        echo unspported Android ABI $1
        return
    fi

    local PLATFORM=${ANDROID_NDK}/platforms/android-${NDK_V}/arch-${CPU_ARCH}
    CC=""${CROSS_COMPILE}"-gcc --sysroot="${PLATFORM}""
    TARGET_OS=Android
    SYSTEM_ROOT=${PLATFORM}

    if [[ "${CPU_ARCH}" =~ "arm" ]]
    then
        NEON_SUPPORT="TRUE"
    else
        NEON_SUPPORT="FALSE"
    fi
}

function android_init_env_clang(){
    if [ "$1" == "armeabi-v7a" ]
    then
        android_armv7_a_init_env
    elif [ "$1" == "arm64-v8a" ]
    then
        android_arm64_v8a_init_env
    else
        echo unspported Android ABI $1
        return
    fi

    clang="${ANDROID_NDK}/toolchains/llvm/prebuilt/darwin-x86_64/bin/clang"

    export NDK_TOOLCHAIN="$ANDROID_NDK/toolchains/$CROSS_COMPILE-4.9/prebuilt/${HOST}-x86_64"

    local PLATFORM=${ANDROID_NDK}/platforms/android-${NDK_V}/arch-${CPU_ARCH}
    CC="$clang -target ${CROSS_COMPILE} -gcc-toolchain ${NDK_TOOLCHAIN} --sysroot="${PLATFORM}""
    TARGET_OS=Android
    SYSTEM_ROOT=${PLATFORM}

    echo "CC is ${CC}"

    if [[ "${CPU_ARCH}" =~ "arm" ]]
    then
        NEON_SUPPORT="TRUE"
    else
        NEON_SUPPORT="FALSE"
    fi
}

