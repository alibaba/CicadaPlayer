#!/usr/bin/env bash
source cross_compile_env.sh
function build_fdk_aac(){
    if [ ! -f ${FDK_AAC_SOURCE_DIR}/configure ]
    then
        cd ${FDK_AAC_SOURCE_DIR}
        ./autogen.sh
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
    elif [ "$1" == "win32" ];then
        cross_compile_set_platform_win32 $2
    elif [ "$1" == "Darwin" ];then
        print_warning "native build for $1"
        local native_build=yes
    else
        echo "Unsupported platform"
        exit 1;
    fi

    local build_dir="build/fdk-aac/$1/$2"
    local install_dir="$PWD/install/fdk-aac/$1/$2"
    mkdir -p ${build_dir}/
     if [ "${BUILD}" != "False" ];then
        cd ${build_dir}
        if [ "$native_build" == "yes" ];then
        ${FDK_AAC_SOURCE_DIR}/configure \
            --enable-static             \
            --disable-shared        \
            --prefix=${install_dir} \
            CFLAGS="${CPU_FLAGS}"   \
            CPPFLAGS="${CPU_FLAGS}" \
            LDFLAGS="${CPU_LDFLAGS}"
        else
        ${FDK_AAC_SOURCE_DIR}/configure \
            --host="${CROSS_COMPILE}"     \
            --enable-static             \
            --disable-shared        \
            --prefix=${install_dir} \
            CC="${CC}"              \
            CXX="$CC"               \
            CPP="$CC -E"            \
            AS="${AS}"              \
            CFLAGS="${CPU_FLAGS}"   \
            CPPFLAGS="${CPU_FLAGS}" \
            LDFLAGS="${CPU_LDFLAGS}"
        fi
        make install -j8 V=1
        cd -
    fi

    export FDK_AAC_INSTALL_DIR=${install_dir}
}

#config_fdk_aac Android armv7-a
#config_fdk_aac Android arm64-v8a