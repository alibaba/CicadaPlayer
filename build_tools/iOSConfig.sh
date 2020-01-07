#!/usr/bin/env bash

function ios_init_globe_env(){
    export DEPLOYMENT_TARGET="8.0"
    IPHONEOS_SDK=$(xcrun --sdk iphoneos --show-sdk-path)

    DEVELOPER=$(xcode-select --print-path)
    IPHONEOS_SDK_VERSION=$(xcrun --sdk iphoneos --show-sdk-version)
    IPHONEOS_PLATFORM=$(xcrun --sdk iphoneos --show-sdk-platform-path)
    IPHONESIMULATOR_PLATFORM=$(xcrun --sdk iphonesimulator --show-sdk-platform-path)
    IPHONESIMULATOR_SDK=$(xcrun --sdk iphonesimulator --show-sdk-path)
}


function ios_init_env(){
    ios_init_globe_env
    local arch
    local ARCH=""
    for arch in ${IOS_ARCHS}
    do
        if [ "$arch" == "$1" ]
        then
            ARCH=$1
        fi
    done

    if [ "$ARCH" == "" ]
    then
        echo unsupported ARCH $1
        exit 1
    fi

    CPU_ARCH=${ARCH}

    CPU_FLAGS="-arch $ARCH"
    CPU_LDFLAGS="-arch $ARCH"
    local PLATFORM
    if [ "$ARCH" = "i386" -o "$ARCH" = "x86_64" ]
    then
        PLATFORM="iPhoneSimulator"
        CPU_FLAGS="$CPU_FLAGS -mios-simulator-version-min=$DEPLOYMENT_TARGET"
        CROSS_COMPILE=${ARCH}-apple-darwin
        ASFLAGS=
        SYSROOT=${IPHONESIMULATOR_SDK}

    else
        PLATFORM="iPhoneOS"
        CPU_FLAGS="$CPU_FLAGS -mfpu=neon -mios-version-min=$DEPLOYMENT_TARGET -fembed-bitcode"
        if [ "$ARCH" = "arm64" ]
        then
            EXPORT="GASPP_FIX_XCODE5=1"
            CROSS_COMPILE=aarch64-apple-darwin
        else
            CROSS_COMPILE=arm-apple-darwin
        fi
        ASFLAGS="$CPU_FLAGS"
        SYSROOT=${IPHONEOS_SDK}
    fi

    local XCRUN_SDK=`echo ${PLATFORM} | tr '[:upper:]' '[:lower:]'`
    CC="xcrun -sdk $XCRUN_SDK clang"
    # force "configure" to use "gas-preprocessor.pl" (FFmpeg 3.3)
    if [ "$ARCH" = "arm64" ]
    then
        AS="gas-preprocessor.pl -arch aarch64 -- $CC"
    else
        AS="gas-preprocessor.pl -- $CC"
    fi

    NEON_SUPPORT="TRUE"
}