#!/usr/bin/env bash

source cross_compille_env.sh

function create_dav1d_config() {
rm -f $user_config
#staring writing file
echo "[binaries]
c     = '${DAV1D_C}'
cpp   = '${DAV1D_CPP}'
ar    = '${DAV1D_AR}'
ld    = '${DAV1D_LD}'
strip = '${DAV1D_STRIP}'

[properties]
c_args = '${DAV1D_ARGS}'
c_link_args = '${DAV1D_ARGS}'
cpp_args = '${DAV1D_ARGS}'
cpp_link_args = '${DAV1D_ARGS}'

[host_machine]
system = '${DAV1D_SYSTEM}'
cpu_family = '${DAV1D_CPU_FAMILY}'
cpu = '${DAV1D_CPU}'
endian = 'little'
" >> ${user_config}

echo "dav1d ${user_config} created"
}

function build_dav1d(){

    local build_dir="build/dav1d/$1/$2"
    local crossfile=""

    local nativeBuild=false
    local cross_opt

    DAV1D_INSTALL_DIR=

    if [[ "$1" == "Android" ]]
    then
        if [[ "$2" != "arm64-v8a" ]]; then
            echo "Only enable arm64-v8a for av1 at present, break $2"
            return;
        fi
        cross_compile_set_platform_Android  $2
        DAV1D_C=""${CROSS_COMPILE}"-gcc"
        DAV1D_CPP=""${CROSS_COMPILE}"-g++"
        DAV1D_AR=""${CROSS_COMPILE}"-ar"
        DAV1D_LD=""${CROSS_COMPILE}"-ld"
        DAV1D_STRIP=""${CROSS_COMPILE}"-strip"
        DAV1D_ARGS="--sysroot=${SYSTEM_ROOT}"
        DAV1D_SYSTEM="linux"
        DAV1D_CPU_FAMILY="aarch64"
        DAV1D_CPU="arm"
    elif [[ "$1" == "iOS" ]]
    then
        if [[ "$2" != "arm64" ]]; then
            echo "Only enable arm64 for av1 at present, break $2"
            return;
        fi
        cross_compile_set_platform_iOS $2
        DAV1D_C="clang"
        DAV1D_CPP="clang++"
        DAV1D_AR="ar"
        DAV1D_LD="ld"
        DAV1D_STRIP="strip"
        DAV1D_ARGS=${CFLAGS}
        DAV1D_SYSTEM="darwin"
        DAV1D_CPU_FAMILY="aarch64"
        DAV1D_CPU="arm"
    elif [[ "$1" == "Darwin" ]]
    then
        echo "native build for $1"
        nativeBuild=true;
    else
        echo "Unsupported platform"
        return;
    fi

    local install_dir="${CWD}/install/dav1d/$1/$2"

    if [[ "${BUILD}" != "False" ]];then
        local build_dir="build/dav1d/$1/$2"
        rm -rf ${build_dir}
        mkdir -p ${build_dir}


        if [[ ${nativeBuild} == false ]]
        then
            echo "Generating toolchain description..."
            user_config=${CWD}/${build_dir}/config.txt
            create_dav1d_config
            cross_opt="--cross-file ${user_config}"
        fi

        #clear env flag due to meson don't fully support
        CFLAGS=
        LDFLAGS=
        CC=

        cd ${build_dir}
        meson ${DAV1D_SOURCE_DIR} --buildtype release --prefix ${install_dir} --default-library static ${cross_opt}
        ninja -C ./
        meson install

        cd ${CWD}
    fi
    DAV1D_INSTALL_DIR=${install_dir}
}
