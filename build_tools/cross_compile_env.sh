#!/usr/bin/env bash

source globeConfig.sh
source AndroidConfig.sh
source iOSConfig.sh
source MinGWConfig.sh
function cross_compile_set_platform_Android(){
    android_init_env $1
}

function cross_compile_set_platform_iOS(){
    ios_init_env $1
}

function cross_compile_set_platform_win32(){
    MinGW_init_env $1
}