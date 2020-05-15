#!/usr/bin/env bash

source cross_compile_env.sh
source native_compile_env.sh
function build_boost() {
    if [ "$1" == "Android" ];then
        cross_compile_set_platform_Android  $2
        export CROSS_COMPILE=${CROSS_COMPILE}
        export NDK_V=${NDK_V}
        export HOST=${HOST}
        export ABI=${ABI}
        export CPU_ARCH=${CPU_ARCH}

       if [ "$ABI" == "armeabi-v7a" ];then
           export ABI_NAME=armeabi_v7a
       elif [ "$ABI" == "arm64-v8a" ];then
           export ABI_NAME=arm64_v8a
    fi
    elif [ "$1" == "iOS" ];then
        cross_compile_set_platform_iOS $2
        export DEVELOPER=${DEVELOPER}
        export IPHONEOS_SDK_VERSION=${IPHONEOS_SDK_VERSION}
        export CPU_ARCH=${CPU_ARCH}
    elif [ "$1" == "Darwin" ];then
       print_warning "native build for $1"
    else
        echo "unsupported platform"
        return 1;
    fi

    if [ "${BUILD}" == "False" ];then
        return 0;
    fi

    rm ~/user-config.jam  #native build not use it
    cp ${BUILD_TOOLS_DIR}/boost/user-config-$1.jam ~/user-config.jam

    local build_dir="$PWD/build/boost/$1/$2"
    local install_dir="$PWD/install/boost/$1/$2"

    cd ${BOOST_SOURCE_DIR}

    [[ -f b2 ]] || sh bootstrap.sh --with-libraries=filesystem,system

    if [ "$1" == "Android" ];then
        ./b2 -a link=static \
        debug-symbols=off \
        optimization=space \
        toolset=gcc-android_${NDK_V}_${ABI_NAME} \
        --build-dir=${build_dir}  \
        --stagedir=${install_dir}

    elif [ "$1" == "iOS" ];then
        if [ "$2" = "i386" -o "$2" = "x86_64" ];then
            ./bjam -j16 \
             --build-dir=${build_dir} \
             --stagedir=${install_dir} \
             --toolset=darwin-${IPHONEOS_SDK_VERSION}~iphonesim \
             architecture=ia64 \
             target-os=iphone \
             macosx-version=iphonesim-${IPHONEOS_SDK_VERSION} \
             link=static \
             stage \
             cxxflags=-miphoneos-version-min=${DEPLOYMENT_TARGET}
        else
            ./bjam -j16  \
            --build-dir=${build_dir} \
            --stagedir=${install_dir} \
            --toolset=darwin architecture=arm \
            target-os=iphone \
            macosx-version=iphone-${IPHONEOS_SDK_VERSION} \
            define=_LITTLE_ENDIAN \
            link=static \
            stage \
            cxxflags="${CPU_FLAGS}"
        fi
    else
        ./b2 --build-dir=${build_dir} --stagedir=${install_dir} link=static stage
    fi

    cd -

}