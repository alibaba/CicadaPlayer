#!/usr/bin/env bash
source cross_compile_env.sh
build_librtmp(){
    local config=
    local shared=no
    if [ "$1" == "Android" ]
    then
        cross_compile_set_platform_Android  $2
        config="$config INC=-I${SYSTEM_ROOT}/usr/include/"
        config="$config XLDFLAGS=-L${SYSTEM_ROOT}/usr/lib/"
        config="$config CROSS_COMPILE="${CROSS_COMPILE}"-"
        local CC=${CC}
        local XCFLAGS="-L${SYSTEM_ROOT}/usr/lib/ -g"
    elif [ "$1" == "iOS" ]
    then
        cross_compile_set_platform_iOS $2
        echo SYSROOT is ${SYSROOT}
        export XCFLAGS="-isysroot ${SYSROOT}  -arch $2 -fembed-bitcode -miphoneos-version-min=$DEPLOYMENT_TARGET"
        export XLDFLAGS="-isysroot ${SYSROOT} -arch $2"
        config="$config CROSS_COMPILE=/usr/bin/"
        config="$config SYS=darwin"
        if [ "$2" == "x86_64" ] || [ "$2" == "i386" ]; then
            export HOST=$2
        else
            export HOST=arm
        fi
    elif [ "$1" == "win32" ];then
        cross_compile_set_platform_win32 $2
        config="$config CROSS_COMPILE="${CROSS_COMPILE}"-"
        local CC=${CC}
        config="$config SYS=mingw"
        export HOST=i686-w64-mingw32
        shared=yes
    elif [ "$1" == "Darwin" ];then
        print_warning "native build for $1"
    else
        echo "Unsupported platform"
        exit 1;
    fi

    local install_dir="$PWD/install/librtmp/$1/$2"
    if [ "${BUILD}" != "False" ];then
        cd ${RTMPDUMP_SOURCE_DIR}/librtmp/
        make clean
        make ${config} XDEF=-DNO_SSL CRYPTO= SHARED=${shared}  prefix=${install_dir} install
        cd -
	fi
	LIBRTMP_INSTALL_DIR=${install_dir}
}