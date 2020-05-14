#!/usr/bin/env bash

function MinGW_init_i686_env(){
    CPU_ARCH=i686
    CROSS_COMPILE=i686-w64-mingw32
    CPU_FLAGS=""
    CPU_LD_FLAGS=""
}

function MinGW_init_x86_64_env(){
    CPU_ARCH=x86_64
    CROSS_COMPILE=x86_64-w64-mingw32
    CPU_FLAGS=""
    CPU_LD_FLAGS=""
}

function MinGW_init_env(){
    if [ "$1" == "i686" ]
    then
        MinGW_init_i686_env
    elif [ "$1" == "x86_64" ]
    then
        MinGW_init_x86_64_env
    else
        echo unspported Windows ABI $1
        return
    fi

    CC=""${CROSS_COMPILE}"-gcc"
    TARGET_OS=mingw32

    NEON_SUPPORT="FALSE"
    HARDENED_CFLAG=""
    echo "mingw not support hardened flags"
}
