#!/usr/bin/env bash

source cross_compile_env.sh

function build_ares(){
    if [ ! -f ${ARES_SOURCE_DIR}/configure ]
    then
        cd ${ARES_SOURCE_DIR}
        ./buildconf
        if [ -n "$MTL" ]
        then
            cp ${BUILD_TOOLS_DIR}/automake_config/config.guess ./
            cp ${BUILD_TOOLS_DIR}/automake_config/config.sub ./
        fi
        cd -
    fi
    if [ "$1" == "Android" ]
    then
        cross_compile_set_platform_Android  $2
    elif [ "$1" == "iOS" ]
    then
        cross_compile_set_platform_iOS $2
        if [ "$2" == "x86_64" ] || [ "$2" == "i386" ]; then
            SYSROOT=${IPHONESIMULATOR_SDK}
        else
            SYSROOT=${IPHONEOS_SDK}
        fi
        export CFLAGS="-arch $2 -fembed-bitcode --sysroot=$SYSROOT -isysroot $SYSROOT -miphoneos-version-min=$DEPLOYMENT_TARGET -g0 -O2"
        export LDFLAGS="-arch $2 --sysroot=$SYSROOT"
        export CC=clang
    elif [ "$1" == "win32" ];then
        cross_compile_set_platform_win32 $2
    elif [ "$1" == "Darwin" ];then
        print_warning "native build for $1"
    else
        echo "Unsupported platform"
        exit 1;
    fi
    local config="--enable-shared=no --disable-symbol-hiding"
    local build_dir="build/cares/$1/$2"
    local install_dir="$PWD/install/cares/$1/$2"
    mkdir -p ${build_dir}/
    if [ "${BUILD}" != "False" ];then
        cd ${build_dir}
        echo "ares check info:${ARES_SOURCE_DIR}/configure --host=${CROSS_COMPILE} CC="${CC}" ${config} --prefix=${install_dir} || exit 1"
        CXXCPP="gcc -E" ${ARES_SOURCE_DIR}/configure --host=${CROSS_COMPILE} CC="${CC}" ${config} --prefix=${install_dir} || exit 1
        make -j8 install V=1 || exit 1
        cd -
    fi
    ARES_INSTALL_DIR=${install_dir}
}
