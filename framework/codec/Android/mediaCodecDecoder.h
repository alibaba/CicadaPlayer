#ifndef QU_ANDROID_H264_DECODER_HH
#define QU_ANDROID_H264_DECODER_HH

#include <cstdio>
#include <list>
#include <jni.h>
#include <thread>
#include <condition_variable>
#include <codec/IDecoder.h>
#include <utils/afThread.h>
#include <base/media/AVAFPacket.h>
#include <queue>
#include <base/media/AFMediaCodecFrame.h>
#include "mediaCodec.h"
#include "mediacodec_jni.h"
#include "codec/ActiveDecoder.h"
#include "../codecPrototype.h"
#include <set>

namespace Cicada{
    class mediaCodecDecoder : public ActiveDecoder, private codecPrototype {
    public:
        mediaCodecDecoder();

        ~mediaCodecDecoder() override;

    private:

        int init_decoder(const Stream_meta *meta, void *wnd, uint64_t flags) override;

        void close_decoder() override;

        int enqueue_decoder(std::unique_ptr<IAFPacket> &pPacket) override;

        int dequeue_decoder(std::unique_ptr<IAFFrame> &pFrame) override;

        void flush_decoder() override;

        int get_decoder_recover_size() override
        {
            return 0;
        };

    private:
        static bool checkSupport(AFCodecID codec, uint64_t flags, int maxSize);


    private:
        explicit mediaCodecDecoder(int dummy)
        {
            addPrototype(this);
        };

        mediaCodecDecoder *clone() override
        {
            return new mediaCodecDecoder();
        };

        bool is_supported(AFCodecID codec, uint64_t flags, int maxSize) override
        {
            if (flags & DECFLAG_HW)
                return checkSupport(codec, flags, maxSize);
            return false;
        };
        static mediaCodecDecoder se;

    private:
        IAFFrame::videoInfo mVideoInfo{};
        mediaCodec *mDecoder{nullptr};

        std::recursive_mutex mFuncEntryMutex;
        bool mbInit{false};

        int mInputFrameCount{0};
        int mOutputFrameCount{0};
        bool mThrowFrame{false};
        bool mUseNdk{false};

        std::mutex mFlushInterruptMuex;
        int mFlushInterrupt{false};

        // mDecoder->flush() state  0: stop  1: flushed 2:Running 3: end-of-stream
        volatile int mFlushState{0};

        std::set<int64_t> mDiscardPTSSet;
    };
}

#endif // QU_ANDROID_H264_DECODER_HH
