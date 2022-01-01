//
// Created by moqi on 2019/9/24.
//

#include "subTitlePacket.h"

subTitlePacket::subTitlePacket(uint8_t *data, int size, int64_t pts, int64_t duration)
{
    mInfo.pts = mInfo.dts = pts;
    mInfo.duration = duration;
    mpBuffer = static_cast<uint8_t *>(malloc(size));
    mSize = size;
    memcpy(mpBuffer, data, size);
    mInfo.flags = 1;
    mInfo.streamIndex = 0;

}

subTitlePacket::~subTitlePacket()
{

    free(mpBuffer);
}

int64_t subTitlePacket::getSize()
{
    return mSize;
}

uint8_t *subTitlePacket::getData()
{
    return mpBuffer;
}

std::unique_ptr<IAFPacket> subTitlePacket::clone() const
{
    return std::unique_ptr<IAFPacket>(new subTitlePacket(mpBuffer, mSize, mInfo.pts, mInfo.duration));
}
