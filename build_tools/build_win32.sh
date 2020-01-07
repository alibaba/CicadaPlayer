#!/usr/bin/env bash

BUILD_TOOLS_DIR=$(cd $(dirname ${BASH_SOURCE[0]}); pwd)

PATH=$PATH:${BUILD_TOOLS_DIR}

source env.sh
source common_build.sh
source utils.sh


CWD=$PWD

echo pwd is $PWD


build_libs win32 "${WIN32_ARCHS}"