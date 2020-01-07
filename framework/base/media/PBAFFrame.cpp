//
// Created by moqi on 2019-08-28.
//
#include "PBAFFrame.h"

extern "C" {
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
}

PBAFFrame::operator AVAFFrame *()
{
    CVReturn err;
    uint8_t *data[4] = {nullptr};
    int linesize[4] = {0};
    int planes, ret, i;
    OSType pixel_format = CVPixelBufferGetPixelFormatType(mPBuffer);

    if (pixel_format != kCVPixelFormatType_420YpCbCr8Planar)
        return nullptr;

    AVFrame *pFrame = av_frame_alloc();
    pFrame->format = AV_PIX_FMT_YUV420P;
    pFrame->width = (int) CVPixelBufferGetWidth(mPBuffer);
    pFrame->height = (int) CVPixelBufferGetHeight(mPBuffer);
    ret = av_frame_get_buffer(pFrame, 32);

    if (ret < 0) {
        av_frame_free(&pFrame);
        return nullptr;
    }

    err = CVPixelBufferLockBaseAddress(mPBuffer, kCVPixelBufferLock_ReadOnly);

    if (err != kCVReturnSuccess) {
        av_frame_free(&pFrame);
        return nullptr;
    }

    if (CVPixelBufferIsPlanar(mPBuffer)) {
        planes = (int)CVPixelBufferGetPlaneCount(mPBuffer);

        for (i = 0; i < planes; i++) {
            data[i] = static_cast<uint8_t *>(CVPixelBufferGetBaseAddressOfPlane(mPBuffer, i));
            linesize[i] = (int)CVPixelBufferGetBytesPerRowOfPlane(mPBuffer, i);
        }
    } else {
        data[0] = static_cast<uint8_t *>(CVPixelBufferGetBaseAddress(mPBuffer));
        linesize[0] = (int)CVPixelBufferGetBytesPerRow(mPBuffer);
    }

    av_image_copy(pFrame->data, pFrame->linesize,
                  (const uint8_t **) data, linesize, static_cast<AVPixelFormat>(pFrame->format),
                  pFrame->width, pFrame->height);
    CVPixelBufferUnlockBaseAddress(mPBuffer, kCVPixelBufferLock_ReadOnly);
    pFrame->pts = mInfo.pts;
    AVAFFrame *pAvFrame = new AVAFFrame(pFrame, FrameTypeVideo);
    av_frame_free(&pFrame);
    return pAvFrame;
}
