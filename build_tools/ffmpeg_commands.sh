#!/bin/bash

source utils.sh
function ffmpeg_config_reset(){
    FFMPEG_DECODER_LIST_ADDED=""
    FFMPEG_ENCODER_LIST_ADDED=""
    FFMPEG_MUXER_LIST_ADDED=""
    FFMPEG_PARSER_LIST_ADDED=""
    FFMPEG_HWACCEL_LIST_ADDED=""
    FFMPEG_PROTOCOL_LIST_ADDED=""
    FFMPEG_BSF_LIST_ADDED=""
    FFMPEG_FILTER_LIST_ADDED=""
    FFMPEG_DEMUXER_LIST_ADDED=""

    ffmpeg_config_user=""

    ffmpeg_decoder_config=""
    ffmpeg_encoder_config=""
    ffmpeg_demuxer_config=""
    ffmpeg_parser_config=""
    ffmpeg_hwaccel_config=""
    ffmpeg_protocol_config=""
    ffmpeg_bsf_config=""
    ffmpeg_filter_config=""
    ffmpeg_cross_compile_config=""
    ffmpeg_extra_cflags=""
    ffmpeg_extra_ldlags=""
    ffmpeg_config_cc=""
    ffmpeg_install_dir=""
}
component_classes="decoder encoder demuxer muxer parser hwaccel protocol bsf filter";

ffmpeg_disable_all_config="--disable-everything \
  --disable-programs\
  --disable-doc \
  --disable-filters \
  --disable-avdevice \
  --disable-postproc \
  --disable-hwaccels \
  --disable-bzlib    \
  --disable-videotoolbox"
ffmpeg_config_default="--enable-avresample --enable-pic"

ffmpeg_config_debug="--disable-optimizations \
  --disable-asm \
  --disable-stripping \
  --enable-debug=3"

ffmpeg_inited=false

function ffmpeg_init_vars(){
    if [[ "${ffmpeg_inited}" == "TRUE" ]]
    then
        echo "inited"
        return
    fi

    if [[ -f ${FFMPEG_SOURCE_DIR}/config.h ]]
    then
        rm ${FFMPEG_SOURCE_DIR}/config.h
    fi
    FFMPEG_DECODER_LIST_SUPPORT=`${FFMPEG_SOURCE_DIR}/configure --list-decoders`
    FFMPEG_ENCODER_LIST_SUPPORT=`${FFMPEG_SOURCE_DIR}/configure --list-encoders`
    FFMPEG_DEMUXER_LIST_SUPPORT=`${FFMPEG_SOURCE_DIR}/configure --list-demuxers`
    FFMPEG_MUXER_LIST_SUPPORT=`${FFMPEG_SOURCE_DIR}/configure   --list-muxers`
    FFMPEG_PARSER_LIST_SUPPORT=`${FFMPEG_SOURCE_DIR}/configure  --list-parsers`
    FFMPEG_HWACCEL_LIST_SUPPORT=`${FFMPEG_SOURCE_DIR}/configure --list-hwaccels`
    FFMPEG_PROTOCOL_LIST_SUPPORT=`${FFMPEG_SOURCE_DIR}/configure --list-protocols`
    FFMPEG_BSF_LIST_SUPPORT=`${FFMPEG_SOURCE_DIR}/configure --list-bsfs`
    FFMPEG_FILTER_LIST_SUPPORT=`${FFMPEG_SOURCE_DIR}/configure --list-filters`
    ffmpeg_inited=TRUE
}

function ffmpeg_config_add_decoders(){
    local i;
    for i in $*; do
        ffmpeg_config_add_component decoder ${i}
        local ret=$?
        if [[ ${ret} -eq 0 ]]
        then
            FFMPEG_DECODER_LIST_ADDED="${FFMPEG_DECODER_LIST_ADDED} ${i}"
        elif [[ ${ret} -eq 2 ]]
        then
            exit 1
        fi
    done
    return 0;
}
function ffmpeg_config_add_encoders(){
    local i;
    for i in $*; do
        ffmpeg_config_add_component encoder ${i}
        if [[ "$?" == "0" ]]
        then
            FFMPEG_ENCODER_LIST_ADDED="${FFMPEG_ENCODER_LIST_ADDED} $i"
        fi
    done
    return 0;
}

function ffmpeg_config_add_demuxers(){
    local i;
    echo $#
    for i in $*; do
#        echo to add demuxer ${i}
        ffmpeg_config_add_component demuxer  ${i}
        local ret=$?
        if [[ $? == 0 ]]
        then
#            echo add demuxer ${i}
            FFMPEG_DEMUXER_LIST_ADDED="${FFMPEG_DEMUXER_LIST_ADDED}  ${i}"
        fi
    done
    return 0;
}

function ffmpeg_config_add_muxers(){
    local i;
    for i in $*; do
        ffmpeg_config_add_component muxer  ${i}
        if [[ $? == 0 ]]
        then
            FFMPEG_MUXER_LIST_ADDED="${FFMPEG_MUXER_LIST_ADDED}  ${i}"
        fi
    done
    return 0;
}

function ffmpeg_config_add_parsers(){
    local i;
    for i in $*; do
        ffmpeg_config_add_component parser  ${i}
        if [[ $? == 0 ]]
        then
            FFMPEG_PARSER_LIST_ADDED="${FFMPEG_PARSER_LIST_ADDED}  ${i}"
        fi
    done
    return 0;
}

function ffmpeg_config_add_bsfs(){
    local i;
    for i in $*; do
        ffmpeg_config_add_component bsf  ${i}
        if [[ $? == 0 ]]
        then
            FFMPEG_BSF_LIST_ADDED="${FFMPEG_BSF_LIST_ADDED} ${i}"
        fi
    done
    return 0;
}

function ffmpeg_config_add_hwaccels(){
    local i;
    for i in $*; do
        ffmpeg_config_add_component hwaccel ${i}
        if [[ $? == 0 ]]
        then
            FFMPEG_HWACCEL_LIST_ADDED="${FFMPEG_HWACCEL_LIST_ADDED} ${i}"
        fi
    done
    return 0;
}

function ffmpeg_config_add_protocols(){
    local i;
    for i in $*; do
 #       echo to add protocol ${i}
        ffmpeg_config_add_component protocol ${i}
        if [[ $? == 0 ]]
        then
#            echo  add protocol ${i}
            FFMPEG_PROTOCOL_LIST_ADDED="${FFMPEG_PROTOCOL_LIST_ADDED} ${i}"
        fi
    done
    return 0;
}

function ffmpeg_config_add_filters(){
    local i;
    for i in $*; do
        ffmpeg_config_add_component filter ${i}
        if [[ $? == 0 ]]
        then
            FFMPEG_FILTER_LIST_ADDED="${FFMPEG_FILTER_LIST_ADDED} ${i}"
        fi
    done
    return 0;
}

function ffmpeg_config_add_component(){
    local component_list="";
    local component_class;
    ffmpeg_check_component_supported $1 $2
    if [[ $? -ne 0 ]]
    then
        print_error "$1 $2 not support"
        return 2
    fi

    for component_class in ${component_classes}
    do
        if [[ "$1" == "${component_class}" ]]
        then
            local component=$(echo ${component_class} | tr '[a-z]' '[A-Z]')
            component_list=FFMPEG_${component}_LIST_ADDED
        fi
    done

    local component
    for component in ${!component_list}
    do
        if [[ "$component" == "$2" ]]
        then
            print_warning "$1 $2 already added"
            return 1;
        fi
    done
#   ${!component_list}="${!component_list} $1"

    return 0;

}

#function ffmpeg_check_decoder_supported(){
#    ffmpeg_check_component_supported decoder $1
#}
#
#function ffmpeg_check_encoder_supported(){
#    ffmpeg_check_component_supported encoder $1
#}
#
#function ffmpeg_check_demuxer_supported(){
#    ffmpeg_check_component_supported demuxer $1
#}
#function ffmpeg_check_muxer_supported(){
#    ffmpeg_check_component_supported muxer $1
#}
#function ffmpeg_check_parser_supported(){
#    ffmpeg_check_component_supported parser $1
#}
#function ffmpeg_check_hwaccel_supported(){
#    ffmpeg_check_component_supported hwaccel $1
#}
#function ffmpeg_check_protocol_supported(){
#    ffmpeg_check_component_supported protocol $1
#}

function ffmpeg_check_component_supported(){
    local component_list="";
    for component_class in ${component_classes}
    do
        if [[ "$1" == "${component_class}" ]]
        then
            local component=$(echo ${component_class} | tr '[a-z]' '[A-Z]')
            component_list=FFMPEG_${component}_LIST_SUPPORT
        fi
    done

    if [[ -z "${!component_list}" ]]
    then
        echo component_class $1 not found
        return -1;
    fi
    local component;

    for component in ${!component_list}
    do
        if [[ "${component}" == "$2" ]]
        then
#            echo support $1 $2
            return 0
        fi
    done
    echo not support $1 $2
    return -1;
}

function ffmpeg_generate_decoder_config(){
    local decoder;
    for decoder in ${FFMPEG_DECODER_LIST_ADDED}
    do
        ffmpeg_decoder_config="${ffmpeg_decoder_config} --enable-decoder=${decoder}"
    done
#    echo ${ffmpeg_decoder_config}
}

function ffmpeg_generate_encoder_config(){
    local encoder;
    for encoder in ${FFMPEG_ENCODER_LIST_ADDED}
    do
        ffmpeg_encoder_config="${ffmpeg_encoder_config} --enable-encoder=${encoder}"
    done
#    echo ${ffmpeg_encoder_config}
}

function ffmpeg_generate_demuxer_config(){
    local demuxer;
    for demuxer in ${FFMPEG_DEMUXER_LIST_ADDED}
    do
        ffmpeg_demuxer_config="${ffmpeg_demuxer_config} --enable-demuxer=${demuxer}"
    done
    echo ${ffmpeg_demuxer_config}
}

function ffmpeg_generate_muxer_config(){
    local muxer;
    for muxer in ${FFMPEG_MUXER_LIST_ADDED}
    do
        ffmpeg_muxer_config="${ffmpeg_muxer_config} --enable-muxer=${muxer}"
    done
#    echo ${ffmpeg_muxer_config}
}

function ffmpeg_generate_parser_config(){
    local parser;
    for parser in ${FFMPEG_PARSER_LIST_ADDED}
    do
        ffmpeg_parser_config="${ffmpeg_parser_config} --enable-parser=${parser}"
    done
#    echo ${ffmpeg_parser_config}
}

function ffmpeg_generate_hwaccel_config(){
    local hwaccel;
    for hwaccel in ${FFMPEG_HWACCEL_LIST_ADDED}
    do
        ffmpeg_hwaccel_config="${ffmpeg_hwaccel_config} --enable-hwaccel=${hwaccel}"
    done
#    echo ${ffmpeg_hwaccel_config}
}

function ffmpeg_generate_protocol_config(){
    local protocol;
    for protocol in ${FFMPEG_PROTOCOL_LIST_ADDED}
    do
        ffmpeg_protocol_config="${ffmpeg_protocol_config} --enable-protocol=${protocol}"
    done
    echo ${ffmpeg_protocol_config}
}

function ffmpeg_generate_filter_config(){
    local filter;
    for filter in ${FFMPEG_FILTER_LIST_ADDED}
    do
        ffmpeg_filter_config="${ffmpeg_filter_config} --enable-filter=${filter}"
    done
#    echo ${ffmpeg_protocol_config}
}

function ffmpeg_generate_bsf_config(){
    local bsf;
    for bsf in ${FFMPEG_BSF_LIST_ADDED}
    do
        ffmpeg_bsf_config="${ffmpeg_bsf_config} --enable-bsf=${bsf}"
    done
#    echo ${ffmpeg_bsf_config}
}

function ffmpeg_config_add_user(){
    ffmpeg_config_user="${ffmpeg_config_user} $1"

}
function ffmpeg_config_set_cross_config(){
    ffmpeg_cross_compile_config="$1"
}
function ffmpeg_config_set_cc(){
    ffmpeg_config_cc="$1"
}
function ffmpeg_config_set_as(){
    ffmpeg_config_as="$1"
}

function ffmpeg_config_add_extra_cflags(){
    if [[ "$1" == "" ]]
    then
        return;
    fi
    if [[ "$ffmpeg_extra_cflags" == "" ]]
    then
        ffmpeg_extra_cflags="$1"
    else
        ffmpeg_extra_cflags="$ffmpeg_extra_cflags $1"
    fi

}
function ffmpeg_config_add_extra_ldflags(){
    if [[ "$1" == "" ]]
    then
        return;
    fi
    if [[ "$ffmpeg_extra_ldlags" == "" ]]
    then
        ffmpeg_extra_ldlags="$1"
    else
        ffmpeg_extra_ldlags="$ffmpeg_extra_ldlags $1"
    fi
}
function ffmpeg_config_set_install(){
    ffmpeg_install_dir=$1

}
function ffmpeg_config(){
    ffmpeg_generate_decoder_config
    ffmpeg_generate_encoder_config
    ffmpeg_generate_demuxer_config
    ffmpeg_generate_muxer_config
    ffmpeg_generate_parser_config
    ffmpeg_generate_bsf_config
    ffmpeg_generate_protocol_config
    ffmpeg_generate_filter_config
    ffmpeg_generate_hwaccel_config

    local component_config="${ffmpeg_decoder_config} \
         ${ffmpeg_encoder_config} \
         ${ffmpeg_demuxer_config} \
         ${ffmpeg_muxer_config}   \
         ${ffmpeg_parser_config}  \
         ${ffmpeg_hwaccel_config} \
         ${ffmpeg_protocol_config} \
         ${ffmpeg_bsf_config}     \
         ${ffmpeg_filter_config}"

    local ff_config="${ffmpeg_disable_all_config}
        ${ffmpeg_config_default}
        ${ffmpeg_config_user}
        ${component_config}
        ${ffmpeg_cross_compile_config}"

    if [[ "${BUILD_TYPE}" == "Debug" ]];then
        ff_config="${ff_config} ${ffmpeg_config_debug}"
    fi

    echo ${ff_config}
    if [[ "${BUILD}" != "False" ]] || [[ "${BUILD_FFMPEG}" != "False" ]];then
        ${FFMPEG_SOURCE_DIR}/configure   ${ff_config} \
            "--as=${ffmpeg_config_as}"                \
            "--cc=${ffmpeg_config_cc}"              \
            "--extra-cflags=${ffmpeg_extra_cflags}"   \
            "--extra-ldflags=${ffmpeg_extra_ldlags}"  \
            --prefix=${ffmpeg_install_dir}

    fi

     FFMPEG_INSTALL_DIR=${ffmpeg_install_dir}
     FFMPEG_BUILD_DIR=$PWD

    return $?
}

function ffmpeg_build(){
    if [[ "${BUILD}" != "False" ]] || [[ "${BUILD_FFMPEG}" != "False" ]];then
        make -j8 V=1
        make install
    fi
}