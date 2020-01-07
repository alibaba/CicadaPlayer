//
// Created by moqi on 2019-07-18.
//

#ifndef FRAMEWORK_BITSTREAMDECODER_H
#define FRAMEWORK_BITSTREAMDECODER_H

#include <functional>
#include <memory>
#include <map>
#include <queue>
#include <demuxer/avFormatDemuxer.h>
#include <codec/avcodecDecoder.h>

namespace Cicada{
    class bitStreamDecoder {
    public:
        typedef int (*data_callback_read)(void *arg, uint8_t *buffer, int size);

        enum eosType {
            eosTypeDemuxer,
            eosTypeDecoder
        };

        class eventListener {
        public:
            virtual void onError(int error) = 0;

            virtual void onEOS(enum eosType type) = 0;

            //        virtual uint8_t *onGetBuffer(int size) = 0;

            virtual void onFrame(IAFFrame *frame, int consumeTime) = 0;

            virtual void onStreamMeta(const Stream_meta *meta) = 0;

            virtual bool onPollData()
            {
                return true;
            };

        };

    public:

        bitStreamDecoder(data_callback_read func, eventListener &listener);

        ~bitStreamDecoder();

        int prepare();

        void pull_once();

        int readPacket();

        void flush();

        int getCacheDuration();

    private:
        struct StatisticsInfo {
            int32_t decoderTimeSend{};
//          int32_t decoderTimeReceive{};
            int64_t lastFramePTS = INT64_MIN;
        };
        int decode(unique_ptr<IAFPacket> &pkt, int index = -1, bool flush = false);

        void addStream(const Stream_meta &meta, int i);

    private:

        typedef std::queue<std::unique_ptr<IAFPacket>> packetQueue;
        avFormatDemuxer mDemuxer{};
        data_callback_read mPRead{};

        std::map<int, packetQueue> mPacketQues{};

        std::map<int, std::unique_ptr<avcodecDecoder>> mDecoders{};
        std::map<int, StatisticsInfo> mStatisInfo{};
        int mVideoIndex{-1};
        int mAudioIndex{-1};
        bool inPutEOS{};
        bool decodersFlushed{};
        bool decoderEOS{};

        eventListener &mListener;

        bool needRead() const;

        int getMinFramePTSStream() const;

        int getMinPacketPTSStream() const;

        void flushDecoders();
    };
}


#endif //FRAMEWORK_BITSTREAMDECODER_H
