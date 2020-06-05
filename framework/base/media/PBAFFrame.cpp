//
// Created by moqi on 2019-08-28.
//
#include "PBAFFrame.h"
#include "AVAFPacket.h"

extern "C" {
#include <libavutil/frame.h>
#include <libavutil/imgutils.h>
}

PBAFFrame::PBAFFrame(CVPixelBufferRef pixelBuffer, int64_t pts, int64_t duration) : mPBuffer(CVPixelBufferRetain(pixelBuffer))
{

    mInfo.pts = pts;
    mInfo.duration = duration;
    mInfo.video.format = AF_PIX_FMT_APPLE_PIXEL_BUFFER;
    mInfo.video.width = (int) CVPixelBufferGetWidth(mPBuffer);
    mInfo.video.height = (int) CVPixelBufferGetHeight(mPBuffer);

    OSType pixel_format = CVPixelBufferGetPixelFormatType(pixelBuffer);
    if (pixel_format == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange) {
        mInfo.video.colorRange = COLOR_RANGE_FULL;
    } else if (pixel_format == kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange) {
        mInfo.video.colorRange = COLOR_RANGE_LIMITIED;
    } else {
        mInfo.video.colorRange = COLOR_RANGE_UNSPECIFIED;
    }

    CFTypeRef colorAttachments = CVBufferGetAttachment((CVPixelBufferRef) pixelBuffer, kCVImageBufferYCbCrMatrixKey, NULL);
    if (colorAttachments != nullptr) {
        if (CFStringCompare((CFStringRef) colorAttachments, kCVImageBufferYCbCrMatrix_ITU_R_601_4, 0) == kCFCompareEqualTo) {
            mInfo.video.colorSpace = COLOR_SPACE_BT601;
        } else if (CFStringCompare((CFStringRef) colorAttachments, kCVImageBufferYCbCrMatrix_ITU_R_709_2, 0) == kCFCompareEqualTo) {
            mInfo.video.colorSpace = COLOR_SPACE_BT709;
        } else if (CFStringCompare((CFStringRef) colorAttachments, kCVImageBufferYCbCrMatrix_ITU_R_2020, 0) == kCFCompareEqualTo) {
            mInfo.video.colorSpace = COLOR_SPACE_BT2020;
        } else {
            mInfo.video.colorSpace = COLOR_SPACE_UNSPECIFIED;
        }
    } else {
        mInfo.video.colorSpace = COLOR_SPACE_UNSPECIFIED;
    }
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