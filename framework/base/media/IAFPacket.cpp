//
// Created by moqi on 2019-07-05.
//
#define LOG_TAG "IAFPacket"

#include "IAFPacket.h"
#include <libavutil/rational.h>
#include <utils/frame_work_log.h>

IAFPacket::packetInfo &IAFPacket::getInfo()
{
    return mInfo;
}

IAFFrame::AFFrameInfo &IAFFrame::getInfo()
{
    return mInfo;
}

void IAFFrame::dump()
{
    const char *type = nullptr;

    switch (getType()) {
        case FrameTypeVideo:
            type = "Video";
            AF_LOGD("frame %p (%s)\n", this, type);
            AF_DUMP_INT(mInfo.video.width);
            AF_DUMP_INT(mInfo.video.height);
            break;

        case FrameTypeAudio:
            type = "Audio";
            AF_LOGD("frame %p (%s)\n", this, type);
            AF_DUMP_INT(mInfo.audio.sample_rate);
            AF_DUMP_INT(mInfo.audio.channels);
            break;

        default:
            type = "Unknown";
    }

    AF_LOGD("pts is %lld\n", this->mInfo.pts);
    //    for (auto &item : getData()) {
    //        AF_LOGD("data ptr is %p", item);
    //    }
    AF_LOGD("\n\n");
}

AFRational &AFRational::operator=(AVRational rational)
{
    num = rational.num;
    den = rational.den;
    return *this;
}

void IAFPacket::packetInfo::dump()
{
    AF_DUMP_INT(streamIndex);
    AF_DUMP_INT(pts);
    AF_DUMP_INT(dts);
    AF_DUMP_INT(duration);
    AF_DUMP_INT(timePosition);
    AF_DUMP_INT(flags);
    AF_LOGD("\n");
}
IAFPacket::packetInfo &IAFPacket::packetInfo::operator=(const IAFPacket::packetInfo &a)
{
    if (this == &a) {
        return *this;
    }
    this->streamIndex = a.streamIndex;
    this->pts = a.pts;
    this->dts = a.dts;
    this->flags = a.flags;
    this->duration = a.duration;
    this->pos = a.pos;
    this->timePosition = a.timePosition;
    this->seamlessPoint = a.seamlessPoint;
    setExtraData(a.extra_data, a.extra_data_size);
    return *this;
}
void IAFPacket::packetInfo::setExtraData(const uint8_t *extraData, int extraDataSize)
{
    if (extraData && extraData != extra_data) {
        delete[] extra_data;
        extra_data = new uint8_t[extraDataSize];
        extra_data_size = extraDataSize;
        memcpy(extra_data, extraData, extra_data_size);
    }
}
