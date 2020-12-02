//
// Created by SuperMan on 2020/10/15.
//
#define LOG_TAG "AdtsBSF"

#include "AdtsBSF.h"
#include <cassert>
#include <libavutil/intreadwrite.h>
#include <utils/frame_work_log.h>

using namespace Cicada;

#define IO_BUFFER_SIZE 32768

AdtsBSF::AdtsBSF()
{}

AdtsBSF::~AdtsBSF()
{
    if (mFormatContext != nullptr) {
        avio_flush(mFormatContext->pb);
        avio_context_free(&mFormatContext->pb);
        avformat_free_context(mFormatContext);
        mFormatContext = nullptr;
    }

    if (mIobuf != nullptr) {
        av_free(mIobuf);
        mIobuf = nullptr;
    }
}

int AdtsBSF::init(const std::string &name, AVCodecParameters *codecpar)
{
    if (name != "aacAdts") {
        return -EINVAL;
    }

    if (codecpar->codec_id != AV_CODEC_ID_AAC) {
        return -EINVAL;
    }

    int ret = avformat_alloc_output_context2(&mFormatContext, nullptr, "adts", nullptr);
    if (ret < 0) {
        AF_LOGE("create adts muxer fail %d", ret);
        return ret;
    }

    mIobuf = (uint8_t *) av_malloc(IO_BUFFER_SIZE);
    mFormatContext->pb = avio_alloc_context(mIobuf, IO_BUFFER_SIZE, AVIO_FLAG_WRITE, this, nullptr, io_write, nullptr);
    mFormatContext->pb->seekable = 0;
    stream = avformat_new_stream(mFormatContext, nullptr);
    ret = avcodec_parameters_copy(stream->codecpar, codecpar);
    if (ret < 0) {
        AF_LOGE("create adts codec par fail %d", ret);
        return ret;
    }

    ret = avformat_write_header(mFormatContext, nullptr);
    if (ret < 0) {
        AF_LOGE("create adts write head fail %d", ret);
        return ret;
    }

    return 0;
}

int AdtsBSF::io_write(void *opaque, uint8_t *buf, int size)
{
    auto *adtsBSF = static_cast<AdtsBSF *>(opaque);
    assert(adtsBSF->targetPkt != nullptr);

    int growBy = size - adtsBSF->targetPkt->size;
    av_grow_packet(adtsBSF->targetPkt, growBy);
    memcpy(adtsBSF->targetPkt->data, buf, size);

    return size;
}

int AdtsBSF::push(AVPacket *pkt)
{
    return 0;
}

int AdtsBSF::pull(AVPacket *pkt)
{
    if (pkt == nullptr) {
        return 0;
    }

    if ((AV_RB16(pkt->data) & 0xfff0) == 0xfff0) {
        //is adts
        return pkt->size;
    }

    targetPkt = pkt;

    int stream_index = pkt->stream_index;
    pkt->stream_index = stream->index;
    av_write_frame(mFormatContext, pkt);
    pkt->stream_index = stream_index;

    return pkt->size;
}