#!/usr/bin/env bash

function native_compile_set_platform_macOS(){
    export DEPLOYMENT_TARGET="10.11"
    CPU_FLAGS="$CPU_FLAGS -mmacosx-version-min=${DEPLOYMENT_TARGET}"
}