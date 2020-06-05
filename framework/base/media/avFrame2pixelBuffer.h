//
// Created by moqi on 2020/6/5.
//

#ifndef CICADAMEDIA_AVFRAME2PIXELBUFFER_H
#define CICADAMEDIA_AVFRAME2PIXELBUFFER_H

#include <CoreVideo/CoreVideo.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <libavutil/frame.h>
CVPixelBufferRef avFrame2pixelBuffer(AVFrame *frame);

#ifdef __cplusplus
};
#endif

#endif//CICADAMEDIA_AVFRAME2PIXELBUFFER_H
