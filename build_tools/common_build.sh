#!/usr/bin/env bash

source build_x264.sh
source build_openssl_111.sh
source build_openssl.sh
source build_curl.sh
source build_dav1d.sh
source build_fdk_aac.sh
source ffmpeg_commands.sh
source build_ffmpeg.sh
source build_librtmp.sh
source build_ares.sh
source build_boost.sh

function apply_ffmpeg_config(){
    ffmpeg_config_reset
    local user_configs=$(cd ${CWD};ls *_ffmpeg_config.sh)
    local user_config
    if [ "$user_configs" == "" ];then
        source ffmpeg_default_config.sh
    else
        for user_config in ${user_configs}
        do
            echo apply ${user_config} to ffmpeg config
            source ${user_config}
        done
    fi
}
function build_static_lib(){
    local arch=$2
    export TARGET_ARCH=$2
    cd ${CWD}

    if [[ -d "$BOOST_SOURCE_DIR" ]];then
        build_boost $1 ${arch}
    else
        print_warning "boost source not found"
    fi

    if [[ -d "${ARES_SOURCE_DIR}" ]];then
        build_ares $1 ${arch}
        if [[ $? -ne 0 ]]; then
            echo "build_ares build failed"
            exit -1
        fi
    else
        print_warning "cares source not found"
    fi

    local build_openssl="true"
    if [[ "$1" == "iOS" ]] || [[ "$1" == "Darwin" ]];then
        if [[ "${SSL_USE_NATIVE}" == "TRUE" ]];then
            build_openssl="false"
        fi
    fi

    if [[ -d "${OPEN_SSL_SOURCE_DIR}" ]] && [[ "${build_openssl}" == "true" ]];then
        if [[  "${OPENSSL_VERSION_111}" == "True" ]];then
            build_openssl_111 $1 ${arch}
        else
            build_openssl $1 ${arch}
        fi
        if [[ $? -ne 0 ]]; then
            echo "build_openssl build failed"
            exit -1
        fi
    else
        print_warning "openssl source not found"
    fi

    if [[ -d "$RTMPDUMP_SOURCE_DIR" ]];then
        build_librtmp $1 ${arch}
        if [[ $? -ne 0 ]]; then
            echo "build_librtmp build failed"
            exit -1
        fi
    else
        print_warning "RTMPDUM source not found"
    fi

    if [[ -d "${CURL_SOURCE_DIR}" ]];then
        build_curl $1 ${arch}
        if [[ $? -ne 0 ]]; then
            echo "build_curl build failed"
            exit -1
        fi
    else
        print_warning "openssl source not found"
    fi
    if [[ -d "${FDK_AAC_SOURCE_DIR}" ]]
    then
        build_fdk_aac $1 ${arch}
        if [[ $? -ne 0 ]]; then
            echo "build_fdk_aac build failed"
            exit -1
        fi
    else
        print_warning "fdk-aac source not found"
    fi
    if [[ -d "${X264_SOURCE_DIR}" ]]
    then
        build_x264 $1 ${arch}
        if [[ $? -ne 0 ]]; then
            echo "build_x264 build failed"
            exit -1
        fi
    else
        print_warning "x264 source not found"
    fi

    if [[ -d "${DAV1D_SOURCE_DIR}" ]]
    then
        cd ${CWD}
        build_dav1d $1 ${arch}
        if [[ $? -ne 0 ]]; then
            echo "build_dav1d build failed"
            exit -1
        fi
    else
        print_warning "dav1d source not found"
    fi

    if [[ -d "${FFMPEG_SOURCE_DIR}" ]]
    then
        cd ${CWD}
        apply_ffmpeg_config
        build_ffmpeg $1 ${arch}
    else
        print_warning "ffmpeg source not found"
    fi
    cd ${CWD}

}
function build_libs(){
    if [[ -d ${FFMPEG_SOURCE_DIR} ]];then
        ffmpeg_init_vars
    fi
    local ARCHS="$2"
    local arch
    for arch in ${ARCHS}
    do
       build_static_lib $1 ${arch}
       if [[ "$1" == "Android" ]];then
           link_shared_lib_Android $1 ${arch}
       fi
    done
    cd ${CWD}

}

function link_shared_lib_Android(){
    if [[ "$1" != "Android" ]];then
        return;
    fi
    local install_dir=${CWD}/install/ffmpeg/Android/$2/
    cross_compile_set_platform_Android  $2
    local cup_arch;
    cup_arch=${CPU_ARCH}
    if [[ "$CPU_ARCH" = arm64 ]]
    then
        cup_arch=aarch64
    fi

    if [[ -z "${LIB_NAME}" ]];then
        export LIB_NAME=alivcffmpeg
    fi

    echo ABI is $2 FFMPEG_BUILD_DIR is $FFMPEG_BUILD_DIR

    local objs="${FFMPEG_BUILD_DIR}/compat/*.o";
    local libraries="libavcodec libswresample libavformat libavutil libswscale libavfilter"
    local library

    for library in ${libraries};
    do
        if [[ -d "${FFMPEG_BUILD_DIR}/${library}/" ]]; then
            objs="${objs} "${FFMPEG_BUILD_DIR}/${library}/*o""
        fi
        if [[ -d "${FFMPEG_BUILD_DIR}/${library}/${cup_arch}" ]]; then
            objs="${objs} "${FFMPEG_BUILD_DIR}/${library}/${cup_arch}/*.o""
        fi
        if [[ -d "${FFMPEG_BUILD_DIR}/${library}/neon" ]]; then
            objs="${objs} "${FFMPEG_BUILD_DIR}/${library}/neon/*.o""
        fi
    done

    local ldflags=""

    if [[ -d "${CURL_INSTALL_DIR}" ]];then
        ldflags="$ldflags -lcurl -L${CURL_INSTALL_DIR}/lib/"
    fi

    if [[ -d "${ARES_INSTALL_DIR}" ]];then
        ldflags="$ldflags -lcares -L${ARES_INSTALL_DIR}/lib/"
    fi

    if [[ -d "${LIBRTMP_INSTALL_DIR}" ]];then
        ldflags="$ldflags -lrtmp -L${LIBRTMP_INSTALL_DIR}/lib/"
    fi

    if [[ -d "${FDK_AAC_INSTALL_DIR}" ]];then
        ldflags="$ldflags -lfdk-aac -L${FDK_AAC_INSTALL_DIR}/lib/"
    fi

    if [[ -d "${OPENSSL_INSTALL_DIR}" ]];then
        ldflags="$ldflags -lssl -lcrypto -L${OPENSSL_INSTALL_DIR}/lib/"
    fi

    if [[ -d "${DAV1D_INSTALL_DIR}" ]];then
        ldflags="$ldflags -ldav1d -L${DAV1D_INSTALL_DIR}/lib/"
    fi

    if [[ -d "${X264_INSTALL_DIR}" ]];then
        ldflags="$ldflags -lx264 -L${X264_INSTALL_DIR}/lib/"
    fi

    cp ${BUILD_TOOLS_DIR}/src/build_version.cpp ./
    sh ${BUILD_TOOLS_DIR}/gen_build_version.sh > version.h

    ${CROSS_COMPILE}-gcc build_version.cpp -lm -lz -shared --sysroot=${SYSTEM_ROOT} \
     -Wl,--no-undefined -Wl,-z,noexecstack ${CPU_LD_FLAGS}  -landroid -llog -Wl,-soname,lib${LIB_NAME}.so \
    ${objs} \
    -o ${install_dir}/lib${LIB_NAME}.so \
    -Wl,--whole-archive   ${ldflags} -Wl,--no-whole-archive

    rm build_version.cpp version.h
}
