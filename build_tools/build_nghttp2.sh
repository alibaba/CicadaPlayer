#!/usr/bin/env bash

function build_nghttp2(){
    if [ ! -f ${NGHTTP2_SOURCE_DIR}/configure ];then
        cd "${NGHTTP2_SOURCE_DIR}" || exit

        if [[ -n "$MTL" ]]
        then
            cp ${BUILD_TOOLS_DIR}/automake_config/config.guess ./
            cp ${BUILD_TOOLS_DIR}/automake_config/config.sub ./
        fi
        autoreconf -fi
        cd - || exit
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
        export CFLAGS="${CFLAGS} -arch $2 -fembed-bitcode --sysroot=$SYSROOT -isysroot $SYSROOT -miphoneos-version-min=$DEPLOYMENT_TARGET"
        export LDFLAGS="-arch $2 --sysroot=$SYSROOT"
        export CC=clang
    elif [[ "$1" == "win32" ]];then
        cross_compile_set_platform_win32 $2
        export CFLAGS="-DNGHTTP2_STATICLIB"
    elif [[ "$1" == "Darwin" ]];then
        print_warning "native build curl for $1 $2"
        native_compile_set_platform_macOS $2
        export CFLAGS="${CFLAGS} $CPU_FLAGS"
    elif [[ "$1" == "Linux" ]];then
        print_warning "native build for $1"
    elif [ "$1" == "maccatalyst" ];then
        cross_compile_set_platform_maccatalyst "$2"
        export CFLAGS="${CPU_FLAGS}"
        export LDFLAGS="${CPU_LDFLAGS}"
        export CC=clang
    else
        echo "curl Unsupported platform $1"
        exit 1;
    fi



    local install_dir="${CWD}/install/nghttp2/$1/$2"

    local build_dir="${CWD}/build/nghttp2/$1/$2"
    mkdir -p "${build_dir}"/

    if [ "${BUILD}" != "False" ];then
        cd "${build_dir}" || exit
        "${NGHTTP2_SOURCE_DIR}"/configure -host=${CROSS_COMPILE} CC="${CC}" --enable-lib-only  --enable-shared=no --prefix=${install_dir}
        make
        make install
        cd - || exit
    fi
    NGHTTP2_INSTALL_DIR=${install_dir}

}