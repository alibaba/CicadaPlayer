#!/usr/bin/env bash
source cross_compille_env.sh

function ffmpeg_cross_compile_config_reset(){
    FFMPEG_CROSS_COMPILE_CONFIG="--enable-cross-compile \
        --enable-small \
        --optflags=-O2 \
        --disable-runtime-cpudetect"
}

function ffmpeg_cross_compile_config_add(){
    FFMPEG_CROSS_COMPILE_CONFIG="${FFMPEG_CROSS_COMPILE_CONFIG} $1"
}
function ffmpeg_cross_compile_set_Android(){
    cross_compile_set_platform_Android "$1";
    ffmpeg_cross_compile_config_add "--target-os=linux"
    ffmpeg_cross_compile_config_add "--arch=${CPU_ARCH}"
    ffmpeg_cross_compile_config_add "--cross-prefix="${CROSS_COMPILE}-""
    ffmpeg_cross_compile_config_add "--disable-linux-perf"
    ffmpeg_cross_compile_config_add "--toolchain=hardened"
    if [ "${NEON_SUPPORT}" == "TRUE" ]
    then
         ffmpeg_cross_compile_config_add "--enable-neon"
         ffmpeg_cross_compile_config_add "--enable-thumb"
    fi
}

function ffmpeg_cross_compile_set_iOS(){
    cross_compile_set_platform_iOS "$1"
    ffmpeg_cross_compile_config_add "--target-os=darwin"
    ffmpeg_cross_compile_config_add "--arch=${CPU_ARCH}"
    if [ "${NEON_SUPPORT}" == "TRUE" ]
    then
         ffmpeg_cross_compile_config_add "--enable-neon"
         ffmpeg_cross_compile_config_add "--enable-thumb"
    fi
    if [ "$1" = "i386" -o "$1" = "x86_64" ]
    then
        ffmpeg_cross_compile_config_add "--disable-asm"
    fi
}

function ffmpeg_cross_compile_set_win32(){
    cross_compile_set_platform_win32 "$1"
    ffmpeg_cross_compile_config_add "--target-os=${TARGET_OS}"
    ffmpeg_cross_compile_config_add "--arch=${CPU_ARCH}"
    ffmpeg_cross_compile_config_add "--cross-prefix="${CROSS_COMPILE}-""

    ffmpeg_cross_compile_config_add "--enable-shared "
    ffmpeg_cross_compile_config_add "--disable-dxva2"
    ffmpeg_cross_compile_config_add "--disable-schannel" #make tls_openssl enable
    if [ "${NEON_SUPPORT}" == "TRUE" ]
    then
         ffmpeg_cross_compile_config_add "--enable-neon"
         ffmpeg_cross_compile_config_add "--enable-thumb"
    fi
}