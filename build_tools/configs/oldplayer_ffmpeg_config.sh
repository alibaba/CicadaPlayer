#!/usr/bin/env bash

ffmpeg_config_add_decoders libfdk_aac h264 mp3 mp3adu mp3float mp3on4float mp3adufloat mp3on4
ffmpeg_config_add_demuxers flv aac live_flv hls webvtt mov mp3 mpegts
ffmpeg_config_add_muxers mp4 adts
ffmpeg_config_add_parsers aac h264
ffmpeg_config_add_bsfs aac_adtstoasc h264_mp4toannexb
ffmpeg_config_add_protocols  rtmp file curl_https curl_http crypto

ffmpeg_config_add_user "--enable-videotoolbox"
ffmpeg_config_add_hwaccels h264_videotoolbox



if [ -n "${CURL_INSTALL_DIR}" ];then
    ffmpeg_config_add_user "--enable-libcurl"
    ffmpeg_config_add_extra_cflags "-I${CURL_INSTALL_DIR}/include"
    ffmpeg_config_add_extra_ldflags "-L${CURL_INSTALL_DIR}/lib"
    ffmpeg_config_add_user "--disable-protocol=http --disable-protocol=https"
fi