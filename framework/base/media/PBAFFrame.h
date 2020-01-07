//
// Created by moqi on 2019-08-28.
//

#ifndef CICADA_PLAYER_PBAFFRAME_H
#define CICADA_PLAYER_PBAFFRAME_H

#include "base/media/IAFPacket.h"
#include <CoreMedia/CoreMedia.h>
#include <utils/AFMediaType.h>
#include <base/media/AVAFPacket.h>

class PBAFFrame : public IAFFrame {
public:
    PBAFFrame(CVPixelBufferRef pixelBuffer, int64_t pts, int64_t duration) :
            mPBuffer(CVPixelBufferRetain(pixelBuffer))
    {

        mInfo.pts = pts;
        mInfo.duration = duration;
        mInfo.format = AF_PIX_FMT_APPLE_PIXEL_BUFFER;
        mInfo.video.width = (int) CVPixelBufferGetWidth(mPBuffer);
        mInfo.video.height = (int) CVPixelBufferGetHeight(mPBuffer);

        OSType pixel_format = CVPixelBufferGetPixelFormatType(pixelBuffer);
        if(pixel_format == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange){
            mInfo.video.colorRange = COLOR_RANGE_FULL;
        } else if(pixel_format == kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange) {
            mInfo.video.colorRange = COLOR_RANGE_LIMITIED;
        } else {
            mInfo.video.colorRange = COLOR_RANGE_UNSPECIFIED;
        }

        CFTypeRef colorAttachments = CVBufferGetAttachment((CVPixelBufferRef)pixelBuffer, kCVImageBufferYCbCrMatrixKey, NULL);
        if (colorAttachments != NULL) {
            if(CFStringCompare((CFStringRef)colorAttachments, kCVImageBufferYCbCrMatrix_ITU_R_601_4, 0) == kCFCompareEqualTo) {
                mInfo.video.colorSpace = COLOR_SPACE_BT601;
            }
            else if(CFStringCompare((CFStringRef)colorAttachments, kCVImageBufferYCbCrMatrix_ITU_R_709_2, 0) == kCFCompareEqualTo) {
                mInfo.video.colorSpace = COLOR_SPACE_BT709;
            }
            else if(CFStringCompare((CFStringRef)colorAttachments, kCVImageBufferYCbCrMatrix_ITU_R_2020, 0) == kCFCompareEqualTo) {
                mInfo.video.colorSpace = COLOR_SPACE_BT2020;
            } else {
                mInfo.video.colorSpace = COLOR_SPACE_UNSPECIFIED;
            }
        } else {
            mInfo.video.colorSpace =  COLOR_SPACE_UNSPECIFIED;
        }

    }

    ~PBAFFrame() override
    {
        CVPixelBufferRelease(mPBuffer);
    }

    FrameType getType() override
    {
        return FrameTypeVideo;
    };

    uint8_t **getData() override
    {
        return nullptr;
    }

    int *getLineSize() override
    {
        return nullptr;
    }

    CVPixelBufferRef getPixelBuffer()
    {
        return mPBuffer;
    }

    std::unique_ptr<IAFFrame> clone() override
    {
        return nullptr;
    }

    operator AVAFFrame *();

private:
    CVPixelBufferRef mPBuffer;


};


#endif //CICADA_PLAYER_PBAFFRAME_H
