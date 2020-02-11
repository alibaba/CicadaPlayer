#!/usr/bin/env bash

ffmpeg_config_add_decoders aac aac_latm h264 hevc mpeg4 mp3 mp3adu mp3float mp3on4float mp3adufloat mp3on4 pcm_s16le ac3_at
ffmpeg_config_add_demuxers flv aac live_flv webvtt mov mp3 mpegts matroska h264
ffmpeg_config_add_muxers mp4 adts mpegts
ffmpeg_config_add_parsers aac h264 hevc aac_latm ac3
ffmpeg_config_add_bsfs aac_adtstoasc h264_mp4toannexb hevc_mp4toannexb extract_extradata
ffmpeg_config_add_protocols file crypto rtmp
ffmpeg_config_add_filters atempo aresample aformat volume

#for ffmpeg concat demuxer
ffmpeg_config_add_demuxers concat

ffmpeg_config_add_protocols http https

if [[ "$TARGET_PLATFORM" != "Android" ]];then
    ffmpeg_config_add_protocols udp
fi

DAV1D_INSTALL_DIR=
if [[ -n "${DAV1D_EXTERNAL_DIR}" ]];then
    if [ -d "${DAV1D_EXTERNAL_DIR}/$TARGET_PLATFORM/$TARGET_ARCH" ];then
        DAV1D_INSTALL_DIR="${DAV1D_EXTERNAL_DIR}/$TARGET_PLATFORM/$TARGET_ARCH"
        ffmpeg_config_add_user "--enable-libdav1d --enable-decoder=libdav1d"
        ffmpeg_config_add_extra_cflags "-I${DAV1D_INSTALL_DIR}/include"
        ffmpeg_config_add_extra_ldflags "-L${DAV1D_INSTALL_DIR}/lib -ldav1d"
    fi
fi

echo "DAV1D_INSTALL_DIR is $DAV1D_INSTALL_DIR"
