#!/usr/bin/env bash

function build_libxml2(){
    if [[ ! -f ${LIBXML2_SOURCE_DIR}/configure ]]
    then
        cd ${LIBXML2_SOURCE_DIR}
        ./autogen.sh
        make distclean
        if [[ -n "$MTL" ]]
        then
            cp ${BUILD_TOOLS_DIR}/automake_config/config.guess ./
            cp ${BUILD_TOOLS_DIR}/automake_config/config.sub ./
        fi
        cd -
    fi

    local shared="false"
    local native_build="no"

    if [[ "$1" == "Android" ]]
    then
        cross_compile_set_platform_Android  $2
    elif [[ "$1" == "iOS" ]]
    then
        cross_compile_set_platform_iOS $2
    elif [[ "$1" == "win32" ]];then
        cross_compile_set_platform_win32 $2
        # only support shared lib for now
        shared="true"
    elif [[ "$1" == "Darwin" ]];then
        print_warning "native build for $1"
        native_build="yes"
    elif [[ "$1" == "Linux" ]];then
        print_warning "native build for $1"
        native_build="yes"
    else
        echo "Unsupported platform"
        exit 1;
    fi

    local shared_opt;

    if [[ "${shared}" == "false" ]];then
        shared_opt="--enable-shared=no"
    else
        shared_opt="--enable-static=no"
    fi

    local build_dir="build/libxml2/$1/$2"
    local install_dir="$PWD/install/libxml2/$1/$2"
    mkdir -p ${build_dir}/
     if [[ "${BUILD}" != "False" ]];then
        cd ${build_dir}
        if [[ "$native_build" == "yes" ]];then
        ${LIBXML2_SOURCE_DIR}/configure \
            ${shared_opt}            \
            --with-pic=yes \
            --without-lzma \
            --without-python \
            --without-debug \
            --without-zlib  \
            --without-iconv \
            --prefix=${install_dir} \
            CFLAGS="${CPU_FLAGS}"   \
            CPPFLAGS="${CPU_FLAGS}" \
            LDFLAGS="${CPU_LDFLAGS}"
        else
        ${LIBXML2_SOURCE_DIR}/configure \
            --host="${CROSS_COMPILE}"     \
            ${shared_opt}            \
            --with-pic=yes  \
            --without-lzma \
            --without-python \
            --without-debug \
            --without-zlib  \
            --without-iconv \
            --prefix=${install_dir} \
            CC="${CC}"              \
            CXX="$CC"               \
            CPP="$CC -E"            \
            AS="${AS}"              \
            CFLAGS="${CPU_FLAGS}"   \
            CPPFLAGS="${CPU_FLAGS}" \
            LDFLAGS="${CPU_LDFLAGS}"
        fi
        make -j8 V=1
        make install
        cd -
    fi

    export LIBXML2_INSTALL_DIR=${install_dir}
}