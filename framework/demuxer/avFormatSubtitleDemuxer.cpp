//
// Created by moqi on 2019/10/30.
//
#define LOG_TAG "avFormatSubtitleDemuxer"

#include <utils/frame_work_log.h>

#include "avFormatSubtitleDemuxer.h"

#include <utils/errors/framework_error.h>
#include <base/media/AVAFPacket.h>

namespace Cicada {
    avFormatSubtitleDemuxer avFormatSubtitleDemuxer::se(0);

    avFormatSubtitleDemuxer::avFormatSubtitleDemuxer()
    {
        mName = LOG_TAG;
        mCtx = avformat_alloc_context();
        mCtx->interrupt_callback.callback = interrupt_cb;
        mCtx->interrupt_callback.opaque = this;
        mCtx->correct_ts_overflow = 0;
        mCtx->flags |= AVFMT_FLAG_KEEP_SIDE_DATA;
    }

    avFormatSubtitleDemuxer::~avFormatSubtitleDemuxer()
    {
        Close();
    }

    int avFormatSubtitleDemuxer::Open()
    {
        if (bOpened) {
            return 0;
        }

        AVInputFormat *in_fmt = nullptr;
        bool use_filename = false;

        if (mReadCb != nullptr) {
            auto *read_buffer = static_cast<uint8_t *>(av_malloc(INITIAL_BUFFER_SIZE));
            // TODO wrap mReadCb, deal SEGEND
            mPInPutPb = avio_alloc_context(read_buffer, INITIAL_BUFFER_SIZE, 0, mUserArg, mReadCb, nullptr, mSeekCb);

            if (mPInPutPb == nullptr) {
                av_free(read_buffer);
                return -ENOMEM;
            }

            mCtx->pb = mPInPutPb;
        } else {
            use_filename = true;
        }

        int ret = avformat_open_input(&mCtx, mPath.c_str(), in_fmt, nullptr);

        if (ret < 0) {
            AF_LOGE("avformat_open_input error %d,%s,", ret,  getErrorString(ret));

            if (ret == AVERROR_PROTOCOL_NOT_FOUND) {
                return FRAMEWORK_ERR_PROTOCOL_NOT_SUPPORT;
            }

            if (ret == AVERROR_EXIT || mInterrupted) {
                return FRAMEWORK_ERR_EXIT;
            }

            return ret;
        }

        bOpened = true;

        do {
            ret = readPacketInternal();
        } while (ret >= 0);

        return 0;
    }

    int avFormatSubtitleDemuxer::GetNbStreams() const
    {
        return mCtx->nb_streams;
    }

    int avFormatSubtitleDemuxer::GetSourceMeta(Source_meta **meta) const
    {
        return 0;
    }

    int avFormatSubtitleDemuxer::GetMediaMeta(Media_meta *mediaMeta) const
    {
        return 0;
    }

    int avFormatSubtitleDemuxer::GetStreamMeta(Stream_meta *meta, int index, bool sub) const
    {
        if (index < 0 || index > mCtx->nb_streams) {
            return -EINVAL;
        }

        get_stream_meta(mCtx->streams[index], meta);

        if (mCtx->duration != AV_NOPTS_VALUE) {
            meta->duration = mCtx->duration;
        } else {
            meta->duration = 0;
        }

        return 0;
    }

    void avFormatSubtitleDemuxer::Start()
    {
    }

    void avFormatSubtitleDemuxer::Stop()
    {
    }

    void avFormatSubtitleDemuxer::flush()
    {
    }

    void avFormatSubtitleDemuxer::interrupt(int inter)
    {
        mInterrupted = static_cast<bool>(inter);
    }

    void avFormatSubtitleDemuxer::Close()
    {
        if (!bOpened) {
            return;
        }

        if (mCtx) {
            avformat_close_input(&mCtx);
        }

        if (mPInPutPb) {
            av_free(mPInPutPb->buffer);
            av_free(mPInPutPb);
            mPInPutPb = nullptr;
        }

        mPacketMap.clear();
        bOpened = false;
    }

    int avFormatSubtitleDemuxer::OpenStream(int index)
    {
        return 0;
    }

    void avFormatSubtitleDemuxer::CloseStream(int index)
    {
    }

    int64_t avFormatSubtitleDemuxer::Seek(int64_t us, int flags, int index)
    {
        mSeekPTS = us;
        return 0;
    }

    int avFormatSubtitleDemuxer::ReadPacket(std::unique_ptr<IAFPacket> &packet, int index)
    {
        // TODO: EOF
        if (mPacketMap.empty()) {
            return -EAGAIN;
        }

        if (mSeekPTS != INT64_MIN) {
            map::iterator item;

            for (item = mPacketMap.begin(); item != mPacketMap.end(); ++item) {
                if (item->second->getInfo().pts + item->second->getInfo().duration >= mSeekPTS) {
                    mCurrent = item;
                    break;
                }
            }

            if (item == mPacketMap.end()) {
                AF_LOGE("seek error\n");
                return -EINVAL;
            }

            mSeekPTS = INT64_MIN;
        }

        if (mCurrentPts == INT64_MIN) {
            mCurrent = mPacketMap.begin();
            mCurrentPts = (*(mCurrent)).second->getInfo().pts;
        }

        if (mCurrent != mPacketMap.end()) {
            packet = ((*(mCurrent)).second->clone());
            mCurrentPts = packet->getInfo().pts;
            mCurrent++;
        } else {
            return 0;
        }

        return static_cast<int>(packet->getSize());
    }

    int avFormatSubtitleDemuxer::readPacketInternal()
    {
        if (!bOpened) {
            return -EINVAL;
        }

        AVPacket *pkt = av_packet_alloc();
        av_init_packet(pkt);
        int err;
        err = av_read_frame(mCtx, pkt);

        if (err < 0) {
            if (err != AVERROR(EAGAIN)) {
                if (mCtx->pb) {
                    av_log(nullptr, AV_LOG_WARNING, "%s:%d: %s, ctx->pb->error=%d\n", __FILE__, __LINE__,  getErrorString(err),
                           mCtx->pb->error);
                }
            }

            if (mCtx->pb && mCtx->pb->error == FRAMEWORK_ERR_EXIT) {
                mCtx->pb->error = 0;
                av_packet_free(&pkt);
                return FRAMEWORK_ERR_EXIT;
            }

            if (err == AVERROR_EOF) {
                if (mCtx->pb && mCtx->pb->error == AVERROR(EAGAIN)) {
                    av_packet_free(&pkt);
                    return mCtx->pb->error;
                }

                if (mCtx->pb && mCtx->pb->error < 0) {
                    int ret = mCtx->pb->error;
                    mCtx->pb->error = 0;
                    av_packet_free(&pkt);
                    return ret;
                }

                av_packet_free(&pkt);
                return AVERROR_EOF;// EOS
            }

            if (err == AVERROR_EXIT) {
                AF_LOGE("AVERROR_EXIT\n");
                av_packet_free(&pkt);
                return -EAGAIN;
            }

            if (err == AVERROR(EAGAIN) || err == SEGEND) {
                if (mCtx->pb) {
                    mCtx->pb->eof_reached = 0;
                    mCtx->pb->error = 0;
                }
            }

            av_packet_free(&pkt);
            return err;
        }

        err = pkt->size;
        pkt->pts = av_rescale_q(pkt->pts, mCtx->streams[pkt->stream_index]->time_base, av_get_time_base_q());
        pkt->dts = av_rescale_q(pkt->dts, mCtx->streams[pkt->stream_index]->time_base, av_get_time_base_q());
        pkt->duration = av_rescale_q(pkt->duration, mCtx->streams[pkt->stream_index]->time_base, av_get_time_base_q());
        int64_t pts = pkt->pts;

        if (!mPacketMap[pts]) {
            mPacketMap[pts] = unique_ptr<IAFPacket>(new AVAFPacket(&pkt));
            mPacketMap[pts]->getInfo().timePosition = mPacketMap[pts]->getInfo().pts;
        } else {
            av_packet_free(&pkt);
        }

        return err;
    }


    bool avFormatSubtitleDemuxer::is_supported(const string &uri, const uint8_t *buffer, int64_t size, int *type,
            const Cicada::DemuxerMeta *meta,
            const Cicada::options *opts)
    {
        unsigned char *pbBuffer = static_cast<unsigned char *>(av_malloc(size + AVPROBE_PADDING_SIZE));
        memcpy(pbBuffer, buffer, size);
        memset(pbBuffer + size, 0, AVPROBE_PADDING_SIZE);
        AVProbeData pd = {uri.c_str(), const_cast<unsigned char *>(pbBuffer), static_cast<int>(size)};
        int score = AVPROBE_SCORE_RETRY;
        AVInputFormat *fmt = av_probe_input_format2(&pd, 1, &score);
        av_freep(&pbBuffer);

        if (fmt && (strcmp(fmt->name, "webvtt") == 0 || strcmp(fmt->name, "srt") == 0 || strcmp(fmt->name, "ass") == 0)) {
            return true;
        };

        return false;
    }

    int avFormatSubtitleDemuxer::interrupt_cb(void *opaque)
    {
        return static_cast<avFormatSubtitleDemuxer *>(opaque)->mInterrupted;
    }

}
