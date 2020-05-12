#!/usr/bin/env bash

#set -x

source ffmpeg_cross_compile_config.sh
source ffmpeg_commands.sh
function build_ffmpeg(){
    ffmpeg_cross_compile_config_reset
    if [[ "$1" == "Android" ]]
    then
        ffmpeg_cross_compile_set_Android  $2
    elif [[ "$1" == "iOS" ]]
    then
        ffmpeg_cross_compile_set_iOS $2
        if [[ "${SSL_USE_NATIVE}" != "TRUE" ]];then
            ffmpeg_config_add_user "--disable-securetransport"
        fi
    elif [[ "$1" == "win32" ]];then
        ffmpeg_cross_compile_set_win32 $2
    elif [[ "$1" == "Darwin" ]];then
        print_warning "native build for $1"
        local native_build=yes
        if [[ "${SSL_USE_NATIVE}" != "TRUE" ]];then
            ffmpeg_config_add_user "--disable-securetransport"
        fi
        ffmpeg_config_add_extra_cflags "-fno-stack-check"
    elif [[ "$1" == "Linux" ]];then
        local native_build=yes
    else
        echo "Unsupported platform"
        return -1;
    fi

    local ffmpeg_build_dir="build/ffmpeg/$1/$2"
    ffmpeg_config_add_extra_cflags "${CPU_FLAGS}"
    ffmpeg_config_add_extra_ldflags "${CPU_LDFLAGS}"
    ffmpeg_config_add_extra_cflags "${HARDENED_CFLAG}"

    if [[ -n "${FDK_AAC_INSTALL_DIR}" ]]; then
        ffmpeg_config_add_user "--enable-libfdk-aac"
        ffmpeg_config_add_extra_cflags "-I${FDK_AAC_INSTALL_DIR}/include"
        ffmpeg_config_add_extra_ldflags "-L${FDK_AAC_INSTALL_DIR}/lib"
    fi

    if [[ -n "${X264_INSTALL_DIR}" ]]; then
        ffmpeg_config_add_user "--enable-libx264"
        ffmpeg_config_add_user "--enable-nonfree --enable-gpl --enable-version3"
        ffmpeg_config_add_extra_cflags "-I${X264_INSTALL_DIR}/include"
        ffmpeg_config_add_extra_ldflags "-L${X264_INSTALL_DIR}/lib"
    fi

    if [[ -n "${OPENSSL_INSTALL_DIR}" ]];then
        ffmpeg_config_add_user "--enable-openssl"
        ffmpeg_config_add_extra_cflags "-I${OPENSSL_INSTALL_DIR}/include"
        ffmpeg_config_add_extra_ldflags "-L${OPENSSL_INSTALL_DIR}/lib"
    fi

    if [[ -d "${DAV1D_INSTALL_DIR}" ]];then
        ffmpeg_config_add_user "--enable-libdav1d --enable-decoder=libdav1d"
        ffmpeg_config_add_extra_cflags "-I${DAV1D_INSTALL_DIR}/include"
        ffmpeg_config_add_extra_ldflags "-L${DAV1D_INSTALL_DIR}/lib -ldav1d"
    fi
    if [[ "$native_build" == "yes" ]];then
        echo native_build
    else
        ffmpeg_config_set_cross_config "${FFMPEG_CROSS_COMPILE_CONFIG}"
        ffmpeg_config_set_cc "$CC"
        ffmpeg_config_set_as "$AS"
    fi

    ffmpeg_config_set_install "$PWD/install/ffmpeg/$1/$2"
    mkdir -p ${ffmpeg_build_dir}
    cd ${ffmpeg_build_dir}
    ffmpeg_config
    if [[ $? -eq 0 ]]
    then
        echo build succese
    else
        echo build error
        exit 1;
    fi
    ffmpeg_build
    if [[ $? -eq 0 ]]
    then
        echo build succese
        return 0;
    else
        echo build error
        exit 1;
    fi
}



