#!/usr/bin/env bash

function native_compile_set_platform_macOS(){
    export DEPLOYMENT_TARGET="10.11"
    CPU_FLAGS="$CPU_FLAGS -mmacosx-version-min=${DEPLOYMENT_TARGET}"
    local arch
    local ARCH=""
    for arch in ${MACOS_ARCHS}
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
    if [ "$ARCH" = "arm64" ];then
      CROSS_COMPILE=arm-apple-darwin
    else
      CROSS_COMPILE=${ARCH}-apple-darwin
    fi

    CC=clang

   if [ "$ARCH" = "arm64" ]
   then
        NEON_SUPPORT="TRUE"
        AS="gas-preprocessor.pl -arch aarch64 -- $CC"
#    else
#        AS="gas-preprocessor.pl -- $CC"
    fi
}