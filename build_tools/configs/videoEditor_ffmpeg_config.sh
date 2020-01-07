#!/usr/bin/env bash

ffmpeg_config_add_decoders amrnb libfdk_aac	pcm_s16le_planar amrwb mjpeg png gif mp3 wavpack h263 mpeg4 h264 pcm_s16le
ffmpeg_config_add_encoders libfdk_aac pcm_s16le libx264
ffmpeg_config_add_parsers  aac mjpeg mpegvideo h263 mpeg4video	png h264 mpegaudio
ffmpeg_config_add_demuxers aac image2 mjpeg amr image2pipe mov flv image_jpeg_pipe mp3 gif image_png_pipe wav
ffmpeg_config_add_muxers   amr mov flv mp4
ffmpeg_config_add_protocols file
ffmpeg_config_add_filters   aformat asetnsamples atempo
ffmpeg_config_add_bsfs      aac_adtstoasc