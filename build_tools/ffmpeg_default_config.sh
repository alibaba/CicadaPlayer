#!/usr/bin/env bash

#ffmpeg_config_add_decoder libfdk_aac
#ffmpeg_config_add_decoder h264
#source build_ffmpeg.sh

ffmpeg_config_add_decoders libfdk_aac h264


ffmpeg_config_add_encoders aac


#ffmpeg_config_add_demuxers flv

ffmpeg_config_add_muxers mp4

ffmpeg_config_add_parsers h264
#ffmpeg_config_add_protocols http

ffmpeg_config_add_bsfs aac_adtstoasc h264_mp4toannexb

ffmpeg_config_add_filters atempo


#ffmpeg_config_add_user --enable-ffplay
#ffmpeg_config_add_user --enable-filter=aresample