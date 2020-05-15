#!/usr/bin/env bash

source cross_compile_env.sh
source utils.sh
function build_openssl(){
##opensll not support config in other dir?
    local config_platform;
    local config_opt;
    if [ "$1" == "Android" ]
    then
        cross_compile_set_platform_Android  $2
        config_platform="android"
        export ANDROID_DEV="$SYSTEM_ROOT/usr"
        print_warning "CROSS_COMPILE is $CROSS_COMPILE"

        local cross_compile_opt="--cross-compile-prefix="${CROSS_COMPILE}"-"
    elif [ "$1" == "iOS" ]
    then
        cross_compile_set_platform_iOS $2
        config_platform="iphoneos-cross"
        CROSS_COMPILE=
        local platform
        local os
        if [ "$2" == "x86_64" ] || [ "$2" == "i386" ]; then
            platform=${IPHONESIMULATOR_PLATFORM}
            os=iPhoneSimulator
        else
            platform=${IPHONEOS_PLATFORM}
            os=iPhoneOS
        fi

        if [ "$2" == "x86_64" ]; then
            config_platform="darwin64-x86_64-cc"
        fi

        export CROSS_TOP="${platform}/Developer"
        export CROSS_SDK="${os}${IPHONEOS_SDK_VERSION}.sdk"
        export BUILD_TOOLS="${DEVELOPER}"
        export CC="${BUILD_TOOLS}/usr/bin/gcc -fembed-bitcode -arch ${2}"
        config_opt="no-dso"

        echo cc is $CC
        #exit 1
    elif [ "$1" == "win32" ];then
        cross_compile_set_platform_win32 $2
        if [ "$2" == "x86_64" ];then
            config_platform="mingw64"
        else
            config_platform="mingw"
        fi
        local cross_compile_opt="--cross-compile-prefix="${CROSS_COMPILE}"-"
    elif [ "$1" == "Darwin" ];then
        print_warning "native build for $1"
        if [ "$2" == "x86_64" ];then
            config_platform="darwin64-x86_64-cc"
        else
            config_platform="darwin-i386-cc"
        fi
    else
        echo "Unsupported platform $1"
        exit 1;
    fi

    local install_dir="$PWD/install/openssl/$1/$2"
    mkdir -p ${install_dir}/lib
    if [ "${BUILD}" != "False" ];then
        cd ${OPEN_SSL_SOURCE_DIR}
        if [ "$1" != "win32" ];then
            sed -ie "s/BIGNUM \*I,/BIGNUM \*i,/g" crypto/rsa/rsa.h
        fi
        ./Configure ${config_platform} ${cross_compile_opt}  ${config_opt} --openssldir=${install_dir}
        make clean

        if [ "$1" == "iOS" ];then
            if [ "$2" == "x86_64" ];then
                sed -ie "s!^CFLAG=!CFLAG=-isysroot ${CROSS_TOP}/SDKs/${CROSS_SDK} -arch $2 -mios-simulator-version-min=${DEPLOYMENT_TARGET} !" "Makefile"
            else
                sed -ie "s!^CFLAG=!CFLAG=-mios-simulator-version-min=${DEPLOYMENT_TARGET} !" "Makefile"
                perl -i -pe 's|static volatile sig_atomic_t intr_signal|static volatile int intr_signal|' crypto/ui/ui_openssl.c
            fi
        fi
        make -j8 V=1 || exit 1 #CALC_VERSIONS="SHLIB_COMPAT=; SHLIB_SOVER=" MAKE="make -e" all -j16
        echo "place-holder make target for avoiding symlinks" >> ${install_dir}/lib/link-shared
        make  install_sw ||exit 1

        cd -
    fi
    OPENSSL_INSTALL_DIR=${install_dir}

}