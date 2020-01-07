//
// Created by moqi on 2019-07-18.
//

#define LOG_TAG "bitStreamDecoder"

#include "bitStreamDecoder.h"
#include <utils/timer.h>
#include <cassert>
#include <utils/mediaFrame.h>

namespace Cicada {

    bitStreamDecoder::bitStreamDecoder(data_callback_read func, eventListener &listener)
        : mPRead(func),
          mListener(listener)
    {
    }

    bitStreamDecoder::~bitStreamDecoder()
    {
        AF_TRACE;
    }

    int bitStreamDecoder::prepare()
    {
        mDemuxer.SetDataCallBack(mPRead, nullptr, nullptr, nullptr, nullptr);
        int ret = mDemuxer.Open();

        if (ret < 0) {
            return ret;
        }

        // TODO: notify info
        Stream_meta meta{};

        for (int i = 0; i < mDemuxer.GetNbStreams(); ++i) {
            mDemuxer.GetStreamMeta(&meta, i, false);
            bool add = false;

            if (meta.type == STREAM_TYPE_VIDEO && mVideoIndex < 0) {
                mVideoIndex = i;
                add = true;
            }

            if (meta.type == STREAM_TYPE_AUDIO && mAudioIndex < 0) {
                mAudioIndex = i;
                add = true;
            }

            if (add) {
                addStream(meta, i);
            }

            releaseMeta(&meta);

            if (mVideoIndex >= 0 && mAudioIndex >= 0) {
                break;
            }
        }

        return 0;
    }

    void bitStreamDecoder::addStream(const Stream_meta &meta, int i)
    {
        mDecoders[i] = unique_ptr<avcodecDecoder>(new avcodecDecoder());
        mDecoders[i]->open(&meta, nullptr, 0);
        mPacketQues[i];
        mStatisInfo[i];
        mDemuxer.OpenStream(i);
        mListener.onStreamMeta(&meta);
    }

    int bitStreamDecoder::decode(unique_ptr<IAFPacket> &pkt, int index, bool flush)
    {
        int64_t decodeTime;

        if (index == -1 && pkt != nullptr) {
            index = pkt->getInfo().streamIndex;
        }

        if (index < 0) {
            return -EINVAL;
        }

        int ret = 0;
        unique_ptr<IAFFrame> frame{};
        decodeTime = af_gettime_relative();

        do {
            ret = mDecoders[index]->getFrame(frame, 0);

            if (ret == 0) {
                mListener.onFrame(frame.get(), (int) (af_gettime_relative() - decodeTime + mStatisInfo[index].decoderTimeSend));
                mStatisInfo[index].decoderTimeSend = 0;
                mStatisInfo[index].lastFramePTS = frame->getInfo().pts;
//               AF_LOGD("%d pts to %lld\n\n", index, mStatisInfo[index].lastFramePTS);
            }
        } while (ret == 0);

        if (ret == STATUS_EOS) {
            AF_LOGD("EOS\n");
            return ret;
        }

        if (ret < 0 && ret != -EAGAIN) {
            AF_LOGE("decoder error\n");
        }

        if (pkt != nullptr || flush) {
            decodeTime = af_gettime_relative();
            ret = mDecoders[index]->send_packet(pkt, 0);
            mStatisInfo[index].decoderTimeSend = af_gettime_relative() - decodeTime;
        }

        return ret;
    }

    int bitStreamDecoder::readPacket()
    {
        unique_ptr<IAFPacket> pkt{};
        int ret = mDemuxer.ReadPacket(pkt, 0);

        if (ret > 0) {
            mPacketQues[pkt->getInfo().streamIndex].push(move(pkt));
        } else if (ret == 0) {
            inPutEOS = true;
            mListener.onEOS(eosTypeDemuxer);
        } else {
            AF_LOGE("read packet error\n");
        }

        return ret;
    }

    int bitStreamDecoder::getMinPacketPTSStream() const
    {
        int64_t dts = INT64_MAX;
        int stream = -1;

        for (auto &item : mPacketQues) {
            if (item.second.empty()) {
                assert(inPutEOS);
                continue;
            }

            if (item.second.front()->getInfo().dts < dts) {
                stream = item.first;
                dts = item.second.front()->getInfo().dts;
            }
        }

        return stream;
    }

    void bitStreamDecoder::pull_once()
    {
        int ret;

        if (decoderEOS) {
            return;
        }

        if (needRead()) {
            AF_LOGI("no data to decoder\n");
            return;
        }

        // get the min pts frame to decode
        int stream = getMinFramePTSStream();
        unique_ptr<IAFPacket> pkt{};
#if !AF_HAVE_PTHREAD

        if (mPacketQues[stream].empty()) {
            assert(inPutEOS);
            stream = getMinPacketPTSStream();
        }

#else
        stream = getMinPacketPTSStream();
#endif

        if (stream == -1) {
            AF_LOGD("PacketQues is empty\n");
            assert(inPutEOS);
            flushDecoders();
            return;
        }

        pkt = move(mPacketQues[stream].front());
        mPacketQues[stream].pop();

        do {
            ret = decode(pkt);
        } while (ret == STATUS_RETRY_IN);
    }

    void bitStreamDecoder::flushDecoders()
    {
        int ret;
        unique_ptr<IAFPacket> pkt{};

        if (!decodersFlushed) {
            for (auto &item : mDecoders) {
                ret = decode(pkt, item.first, true);
            }

            decodersFlushed = true;
        } else if (!decoderEOS) {
            for (auto &item : mDecoders) {
                do {
                    ret = decode(pkt, item.first);
                } while (ret == -EAGAIN);
            }

            decoderEOS = true;
            mListener.onEOS(eosTypeDecoder);
        }
    }

    int bitStreamDecoder::getMinFramePTSStream() const
    {
        int stream = -1;
        {
            int64_t pts = INT64_MAX;

            for (auto &item : mStatisInfo) {
                //       AF_LOGD("%d pts is %lld\n", item.first, item.second.lastFramePTS);
                if (item.second.lastFramePTS == INT64_MIN) {
                    stream = item.first;
                    break;
                }

                if (item.second.lastFramePTS == INT64_MIN || item.second.lastFramePTS < pts) {
                    stream = item.first;
                    pts = item.second.lastFramePTS;
                }
            }
        }
        //   AF_LOGD("stream is %d\n", stream);
        return stream;
    }

    bool bitStreamDecoder::needRead() const
    {
        if (inPutEOS) {
            return false;
        }

        for (auto &item : mPacketQues) {
            if (item.second.empty()) {
                return true;
            }
        }

        return false;
    }

    void bitStreamDecoder::flush()
    {
        for (auto &item : mPacketQues) {
            while (!item.second.empty()) {
                item.second.pop();
            }
        }

        mDemuxer.flush();

        for (auto &item : mDecoders) {
            item.second->flush();
        }

        inPutEOS = false;
        decoderEOS = false;
        decodersFlushed = false;
    }

    int bitStreamDecoder::getCacheDuration()
    {
        int duration = 0;

        for (auto &item : mPacketQues) {
            if (item.second.empty()) {
                return 0;
            }

            if (item.second.back()->getInfo().dts != INT64_MIN && item.second.front()->getInfo().dts != INT64_MIN) {
                duration = item.second.back()->getInfo().dts - item.second.front()->getInfo().dts;
            }
        }

        return duration;
    }
}
