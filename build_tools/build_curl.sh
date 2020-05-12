#!/usr/bin/env bash

source cross_compille_env.sh

function build_curl(){
    if [ ! -f ${CURL_SOURCE_DIR}/configure ]
    then
        cd ${CURL_SOURCE_DIR}
        ./buildconf
        if [ -n "$MTL" ]
        then
            cp ${BUILD_TOOLS_DIR}/automake_config/config.guess ./
            cp ${BUILD_TOOLS_DIR}/automake_config/config.sub ./
        fi
        cd -
    fi

    LIBSDEPEND=""

    local ssl_opt=

    export CFLAGS="${HARDENED_CFLAG}"

    if [ "$1" == "Android" ]
    then
        cross_compile_set_platform_Android  $2
    elif [ "$1" == "iOS" ]
    then
        LIBSDEPEND="LIBS=-lresolv"
        cross_compile_set_platform_iOS $2
        if [ "$2" == "x86_64" ] || [ "$2" == "i386" ]; then
            SYSROOT=${IPHONESIMULATOR_SDK}
        else
            SYSROOT=${IPHONEOS_SDK}
        fi
        export CFLAGS="${CFLAGS} -arch $2 -fembed-bitcode --sysroot=$SYSROOT -isysroot $SYSROOT -miphoneos-version-min=$DEPLOYMENT_TARGET"
        export LDFLAGS="-arch $2 --sysroot=$SYSROOT"
        export CC=clang
        if [[ "${SSL_USE_NATIVE}" == "TRUE" ]];then
            ssl_opt="--with-darwinssl"
        fi
    elif [[ "$1" == "win32" ]];then
        cross_compile_set_platform_win32 $2
    elif [[ "$1" == "Darwin" ]];then
        LIBSDEPEND="LIBS=-lresolv"
        if [[ "${SSL_USE_NATIVE}" == "TRUE" ]];then
            ssl_opt="--with-darwinssl"
        fi
        print_warning "native build for $1"
    elif [[ "$1" == "Linux" ]];then
        LIBSDEPEND="LIBS=-lresolv"
        print_warning "native build for $1"
    else
        echo "Unsupported platform"
        exit 1;
    fi

# \
    local config="--enable-shared=no \
                --disable-symbol-hiding \
                --enable-proxy \
                --disable-debug \
                --enable-optimize \
                --disable-ftp \
                --disable-gopher \
                --disable-file \
                --disable-imap \
                --disable-ldap \
                --disable-ldaps \
                --disable-pop3 \
                --disable-rtsp \
                --disable-smtp \
                --disable-telnet \
                --disable-tftp \
                --disable-smb \
                --disable-smbs \
                --disable-dict \
                --without-gnutls \
                --without-libidn2 \
                --without-librtmp \
                --without-libidn"
    local build_dir="${CWD}/build/curl/$1/$2"
    local install_dir="${CWD}/install/curl/$1/$2"

    mkdir -p ${build_dir}/
    if [ "${BUILD}" != "False" ];then
        cd ${build_dir}
        if [ -z "${ssl_opt}" ] && [ -d "${OPENSSL_INSTALL_DIR}" ];then
            local ssl_opt="--with-ssl=${OPENSSL_INSTALL_DIR}"
        fi

        if [ -d "${ARES_INSTALL_DIR}" ];then
            local resolver_opt="--enable-ares=${ARES_INSTALL_DIR}"
        else
            local resolver_opt="--enable-threaded-resolver"
        fi

        if [ -d "${LIBRTMP_INSTALL_DIR}" ];then
            local rtmp_opt="--with-librtmp=${LIBRTMP_INSTALL_DIR}"
        fi

        ${CURL_SOURCE_DIR}/configure -host=${CROSS_COMPILE} CC="${CC}" ${ssl_opt} ${resolver_opt} ${config} ${rtmp_opt} --prefix=${install_dir} ${LIBSDEPEND} || exit 1
        make -j8 install V=1 || exit 1
        cd -
    fi
    CURL_INSTALL_DIR=${install_dir}
    export CFLAGS=""
}
