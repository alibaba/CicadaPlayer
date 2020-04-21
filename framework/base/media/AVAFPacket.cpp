//
// Created by moqi on 2019-07-05.
//
#include <utils/frame_work_log.h>
#include <cassert>
#include "base/media/IAFPacket.h"
#include "AVAFPacket.h"
#include "utils/ffmpeg_utils.h"

using namespace std;

void AVAFPacket::copyInfo()
{
    mInfo.duration = mpkt->duration;
    mInfo.pts = mpkt->pts;
    mInfo.dts = mpkt->dts;
    // TODO: redefine the flags
    mInfo.flags = 0;
    if (mpkt->flags & AV_PKT_FLAG_KEY) {
        mInfo.flags |= AF_PKT_FLAG_KEY;
    }
    if (mpkt->flags & AV_PKT_FLAG_CORRUPT) {
        mInfo.flags |= AF_PKT_FLAG_CORRUPT;
    }
    if (mpkt->flags & AV_PKT_FLAG_DISCARD) {
        setDiscard(true);
    }

    mInfo.streamIndex = mpkt->stream_index;
    mInfo.timePosition = INT64_MIN;
    mInfo.pos = mpkt->pos;
}

AVAFPacket::AVAFPacket(AVPacket &pkt)
{
    mpkt = av_packet_alloc();
    av_init_packet(mpkt);
    av_packet_ref(mpkt, &pkt);
    copyInfo();
}

AVAFPacket::AVAFPacket(AVPacket *pkt)
{
    mpkt = av_packet_alloc();
    av_init_packet(mpkt);
    av_packet_ref(mpkt, pkt);
    copyInfo();
}

AVAFPacket::AVAFPacket(AVPacket **pkt)
{
    mpkt = *pkt;
    *pkt = nullptr;
    copyInfo();
}

AVAFPacket::~AVAFPacket()
{
    av_packet_free(&mpkt);
}

uint8_t *AVAFPacket::getData()
{
    return mpkt->data;
}

unique_ptr<IAFPacket> AVAFPacket::clone()
{
    return unique_ptr<IAFPacket>(new AVAFPacket(mpkt));
}

int64_t AVAFPacket::getSize()
{
    return mpkt->size;
}

AVPacket *AVAFPacket::ToAVPacket()
{
    return mpkt;
}

void AVAFPacket::setDiscard(bool discard)
{
    if (discard) {
        mpkt->flags |= AV_PKT_FLAG_DISCARD;
    } else {
        mpkt->flags &= ~AV_PKT_FLAG_DISCARD;
    }

    IAFPacket::setDiscard(discard);
}

AVAFPacket::operator AVPacket *()
{
    return mpkt;
}

AVAFFrame::AVAFFrame(AVFrame **frame, IAFFrame::FrameType type) : mType(type)
{
    assert(*frame != nullptr);
    mAvFrame = *frame;
    *frame = nullptr;
    copyInfo();
}


AVAFFrame::AVAFFrame(AVFrame *frame, FrameType type) : mAvFrame(av_frame_clone(frame)),
                                                       mType(type)
{
    assert(mAvFrame != nullptr);
    copyInfo();
}

void AVAFFrame::copyInfo()
{
    if (mType == FrameTypeUnknown) {
        mType = getType();
    }

    mInfo.pts = mAvFrame->pts;
    mInfo.pkt_dts = mAvFrame->pkt_dts;
    mInfo.key = mAvFrame->key_frame;
    mInfo.duration = mAvFrame->pkt_duration;

    if (mType == FrameTypeVideo) {
        mInfo.video.height = mAvFrame->height;
        mInfo.video.width = mAvFrame->width;
        mInfo.video.sample_aspect_ratio = mAvFrame->sample_aspect_ratio;
        mInfo.video.crop_bottom = mAvFrame->crop_bottom;
        mInfo.video.crop_left = mAvFrame->crop_left;
        mInfo.video.crop_right = mAvFrame->crop_right;
        mInfo.video.crop_top = mAvFrame->crop_top;
        mInfo.video.colorSpace = AVColorSpace2AF(mAvFrame->colorspace);
        mInfo.video.colorRange = AVColorRange2AF(mAvFrame->color_range);
        mInfo.format = AVPixFmt2Cicada((enum AVPixelFormat) mAvFrame->format);
    } else if (mType == FrameTypeAudio) {
        mInfo.audio.channels = mAvFrame->channels;
        mInfo.audio.nb_samples = mAvFrame->nb_samples;
        mInfo.audio.channel_layout = mAvFrame->channel_layout;
        mInfo.audio.sample_rate = mAvFrame->sample_rate;
        mInfo.audio.format = mInfo.format = (enum AFSampleFormat) mAvFrame->format;
    }
}

AVAFFrame::~AVAFFrame()
{
    av_frame_free(&mAvFrame);
}

uint8_t **AVAFFrame::getData()
{
    // TODO: get the data count for video and audio
    // vector<uint8_t *> data{mAvFrame->data[0], mAvFrame->data[1], mAvFrame->data[2],};
    return mAvFrame->data;
}

int *AVAFFrame::getLineSize()
{
    // TODO: get the line size count for video and audio
    return mAvFrame->linesize;
    // return vector<int>{mAvFrame->linesize[0], mAvFrame->linesize[1], mAvFrame->linesize[2],};
}

IAFFrame::FrameType AVAFFrame::getType()
{
    if (mType != FrameTypeUnknown) {
        return mType;
    }

    if (mAvFrame->width > 0 && mAvFrame->height > 0) {
        return FrameTypeVideo;
    }

    if (mAvFrame->nb_samples > 0 && mAvFrame->channels > 0) {
        return FrameTypeAudio;
    }

    return FrameTypeUnknown;
}

std::unique_ptr<IAFFrame> AVAFFrame::clone()
{
    return unique_ptr<IAFFrame>(new AVAFFrame(mAvFrame));
}

AVFrame *AVAFFrame::ToAVFrame()
{
    return mAvFrame;
}

AVAFFrame::operator AVFrame *() const
{
    return mAvFrame;
}

void AVAFFrame::updateInfo()
{
    copyInfo();
}

