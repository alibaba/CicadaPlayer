#!/usr/bin/env bash

source cross_compile_env.sh
function build_x264(){
    if [ "$1" == "Android" ]
    then
        cross_compile_set_platform_Android  $2
    elif [ "$1" == "iOS" ]
    then
        cross_compile_set_platform_iOS $2
    elif [ "$1" == "win32" ];then
        cross_compile_set_platform_win32 $2
    elif [ "$1" == "Darwin" ];then
        print_warning "native build for $1"
        CC=gcc
    else
        echo "Unsupported platform"
        exit 1;
    fi

    export -n AS

    local build_dir="build/x264/$1/$2"
    local install_dir="$PWD/install/x264/$1/$2"
    local config="--enable-pic
                  --enable-static
                  --disable-cli
                  --prefix=${install_dir}
                  --host="${CROSS_COMPILE}""

    if [[ "$1" == "Android" ]];then
        config="${config}  --cross-prefix="${CROSS_COMPILE}"-"
    elif [[ "$1" == "iOS" ]];then
        config="${config} --disable-opencl"
    fi



    mkdir -p ${build_dir}/
    if [ "${BUILD}" != "False" ];then
        cd ${build_dir}
        CC=$CC ${X264_SOURCE_DIR}/configure  \
            ${config}                        \
            --extra-asflags="${ASFLAGS}" \
            --extra-cflags="${CPU_FLAGS}"   \
            --extra-ldflags="${CPU_LDFLAGS}"

        [ "$1" == "Android" ] && [ "${NDK_V}" -lt 19 ] && (sed -i -- 's/HAVE_LOG2F 1/HAVE_LOG2F 0/g' config.h)
        if [ "$1" == "iOS" ]
        then
            local GAS_PREPROCESSOR=$(which gas-preprocessor.pl)
            mkdir extras
            ln -s ${GAS_PREPROCESSOR} extras
        fi

        make install -j8 V=1
        cd -
    fi

    export X264_INSTALL_DIR=${install_dir}
}