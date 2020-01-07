//
// Created by moqi on 2019-08-14.
//

#ifndef CICADA_PLAYER_AFMEDIACODECFRAME_H
#define CICADA_PLAYER_AFMEDIACODECFRAME_H


#include "IAFPacket.h"
#include <utils/AFMediaType.h>

#include <utility>
#include <functional>

class AFMediaCodecFrame : public IAFFrame {
public:
    typedef std::function<void(int, bool)> release;

    AFMediaCodecFrame(FrameType type, int index, release f_release);


    void setDiscard(bool discard) override;


    uint8_t **getData() override
    {
        return nullptr;
    }

    int *getLineSize() override
    {
        return nullptr;
    }

    FrameType getType() override
    {
        return mType;
    }

    std::unique_ptr<IAFFrame> clone() override
    {
        return nullptr;
    }

    ~AFMediaCodecFrame() override;

private:
    int mIndex = -1;
    FrameType mType;
    bool mDiscard{false};
    release mRelease;

};


#endif //CICADA_PLAYER_AFMEDIACODECFRAME_H
