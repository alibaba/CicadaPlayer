//
// Created by moqi on 2019-07-05.
//
#include <utils/frame_work_log.h>
#include <cassert>
#include "base/media/IAFPacket.h"
#include "AVAFPacket.h"
#include "utils/ffmpeg_utils.h"
#ifdef __APPLE__
#include "PBAFFrame.h"
#include "avFrame2pixelBuffer.h"
#endif

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

AVAFPacket::AVAFPacket(AVPacket &pkt, bool isProtected) : mIsProtected(isProtected)
{
    mpkt = av_packet_alloc();
    av_init_packet(mpkt);
    av_packet_ref(mpkt, &pkt);
    copyInfo();
}

AVAFPacket::AVAFPacket(AVPacket *pkt, bool isProtected) : mIsProtected(isProtected)
{
    mpkt = av_packet_alloc();
    av_init_packet(mpkt);
    av_packet_ref(mpkt, pkt);
    copyInfo();
}

AVAFPacket::AVAFPacket(AVPacket **pkt, bool isProtected) : mIsProtected(isProtected)
{
    mpkt = *pkt;
    *pkt = nullptr;
    copyInfo();
}

AVAFPacket::~AVAFPacket()
{
    if (mAVEncryptionInfo != nullptr) {
        av_encryption_info_free(mAVEncryptionInfo);
    }

    av_packet_free(&mpkt);
}

uint8_t *AVAFPacket::getData()
{
    return mpkt->data;
}

unique_ptr<IAFPacket> AVAFPacket::clone() const
{
    return unique_ptr<IAFPacket>(new AVAFPacket(mpkt, mIsProtected));
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

bool AVAFPacket::getEncryptionInfo(IAFPacket::EncryptionInfo *dst)
{
    if(mAVEncryptionInfo == nullptr) {

        int encryption_info_size;
        const uint8_t *new_encryption_info = av_packet_get_side_data(mpkt, AV_PKT_DATA_ENCRYPTION_INFO, &encryption_info_size);

        if (encryption_info_size <= 0 || new_encryption_info == nullptr) {
            return false;
        }

        mAVEncryptionInfo = av_encryption_info_get_side_data(new_encryption_info, encryption_info_size);
    }

    if (mAVEncryptionInfo == nullptr) {
        return false;
    }

    if (mAVEncryptionInfo->scheme == MKBETAG('c', 'e', 'n', 'c')) {
        dst->scheme = "cenc";
    } else if (mAVEncryptionInfo->scheme == MKBETAG('c', 'e', 'n', 's')) {
        dst->scheme = "cens";
    } else if (mAVEncryptionInfo->scheme == MKBETAG('c', 'b', 'c', '1')) {
        dst->scheme = "cbc1";
    } else if (mAVEncryptionInfo->scheme == MKBETAG('c', 'b', 'c', 's')) {
        dst->scheme = "cbcs";
    }

    dst->crypt_byte_block = mAVEncryptionInfo->crypt_byte_block;
    dst->skip_byte_block = mAVEncryptionInfo->skip_byte_block;
    dst->subsample_count = mAVEncryptionInfo->subsample_count;

    dst->key_id = mAVEncryptionInfo->key_id;
    dst->key_id_size = mAVEncryptionInfo->key_id_size;

    dst->iv = mAVEncryptionInfo->iv;
    dst->iv_size = mAVEncryptionInfo->iv_size;

    if (mAVEncryptionInfo->subsample_count > 0) {
        dst->subsample_count = mAVEncryptionInfo->subsample_count;
        for(int i = 0; i < mAVEncryptionInfo->subsample_count; i++) {
            SubsampleEncryptionInfo subInfo{};
            subInfo.bytes_of_protected_data =  mAVEncryptionInfo->subsamples[i].bytes_of_protected_data;
            subInfo.bytes_of_clear_data = mAVEncryptionInfo->subsamples[i].bytes_of_clear_data;
            dst->subsamples.push_back(subInfo);
        }
    } else {
        dst->subsample_count = 1;
        SubsampleEncryptionInfo subInfo{};
        subInfo.bytes_of_protected_data = getSize();
        subInfo.bytes_of_clear_data = 0;
        dst->subsamples.push_back(subInfo);
    }

    return true;
}


AVAFFrame::AVAFFrame(const IAFFrame::AFFrameInfo &info, const uint8_t **data, const int *lineSize, int lineNums, IAFFrame::FrameType type)
    : mType(type)
{
    AVFrame *avFrame = av_frame_alloc();
    if (type == FrameType::FrameTypeAudio) {
        audioInfo aInfo = info.audio;
        avFrame->channels = aInfo.channels;
        avFrame->sample_rate = aInfo.sample_rate;
        avFrame->format = aInfo.format;
        int sampleSize = av_get_bytes_per_sample((enum AVSampleFormat)(avFrame->format));
        avFrame->nb_samples = (int) (lineSize[0] / (avFrame->channels * sampleSize));
    } else if (type == FrameType::FrameTypeVideo) {
        videoInfo vInfo = info.video;
        avFrame->width = vInfo.width;
        avFrame->height = vInfo.height;
        avFrame->format = vInfo.format;
    }

    av_frame_get_buffer(avFrame, 32);
    av_frame_make_writable(avFrame);
    for (int i = 0; i < lineNums; i++) {
        uint8_t *frameSamples = avFrame->data[i];
        memcpy(frameSamples, data[i], lineSize[i]);
    }

    mAvFrame = avFrame;
    copyInfo();
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
        mInfo.video.format = AVPixFmt2Cicada((enum AVPixelFormat) mAvFrame->format);
    } else if (mType == FrameTypeAudio) {
        mInfo.audio.channels = mAvFrame->channels;
        mInfo.audio.nb_samples = mAvFrame->nb_samples;
        mInfo.audio.channel_layout = mAvFrame->channel_layout;
        mInfo.audio.sample_rate = mAvFrame->sample_rate;
        mInfo.audio.format = (enum AFSampleFormat) mAvFrame->format;
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
#ifdef __APPLE__
AVAFFrame::operator PBAFFrame *()
{
    CVPixelBufferRef pixelBuffer = avFrame2pixelBuffer(mAvFrame);
    if (pixelBuffer) {
        auto* frame = new PBAFFrame(pixelBuffer, mInfo.pts, mInfo.duration);
        CVPixelBufferRelease(pixelBuffer);
        return frame;
    }
    return nullptr;
}
#endif
