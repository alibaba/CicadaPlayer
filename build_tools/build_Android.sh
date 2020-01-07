#!/usr/bin/env bash
BUILD_TOOLS_DIR=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)

PATH=$PATH:${BUILD_TOOLS_DIR}

source env.sh
source common_build.sh
source utils.sh


CWD=$PWD

print_warning "build Android"
if [ -d "${ANDROID_NDK}" ];then
    echo ANDROID_NDK is ${ANDROID_NDK}
else
    print_error "ANDROID_NDK not set"
    exit 1
fi
build_libs Android "${ANDROID_ABIS}"
