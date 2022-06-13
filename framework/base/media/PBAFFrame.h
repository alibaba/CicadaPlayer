//
// Created by moqi on 2019-08-28.
//

#ifndef CICADA_PLAYER_PBAFFRAME_H
#define CICADA_PLAYER_PBAFFRAME_H

#include "base/media/IAFPacket.h"
#include <CoreMedia/CoreMedia.h>
#include <utils/AFMediaType.h>

class AVAFFrame;

class CICADA_CPLUS_EXTERN PBAFFrame : public IAFFrame {
public:
    PBAFFrame(CVPixelBufferRef pixelBuffer, int64_t pts, int64_t duration, const VideoColorInfo &info);

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
        mData[0] = reinterpret_cast<uint8_t *>(mPBuffer);
        return mData;
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

    explicit operator AVAFFrame *();

private:
    CVPixelBufferRef mPBuffer;
    uint8_t *mData[1];
};


#endif //CICADA_PLAYER_PBAFFRAME_H
