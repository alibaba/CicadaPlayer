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
#include <set>

#include <base/media/AFMediaCodecFrame.h>
#include <drm/WideVineDrmHandler.h>
#include "codec/ActiveDecoder.h"
#include "../codecPrototype.h"
#include "jni/MediaCodec_Decoder.h"


#define CODEC_VIDEO (0)
#define CODEC_AUDIO (1)

namespace Cicada{
    class mediaCodecDecoder : public ActiveDecoder, private codecPrototype {
    public:
        mediaCodecDecoder();

        ~mediaCodecDecoder() override;

    private:

        int init_decoder(const Stream_meta *meta, void *wnd, uint64_t flags, const DrmInfo *drmInfo) override;

        void close_decoder() override;

        int enqueue_decoder(std::unique_ptr<IAFPacket> &pPacket) override;

        int dequeue_decoder(std::unique_ptr<IAFFrame> &pFrame) override;

        void flush_decoder() override;

        int get_decoder_recover_size() override
        {
            return 0;
        };

    private:
        static bool checkSupport(const Stream_meta &meta, uint64_t flags, int maxSize);

        int setCSD(const Stream_meta *meta);

        int initDrmHandler();

        void releaseDecoder();

        int configDecoder();

    private:
        explicit mediaCodecDecoder(int dummy)
        {
            addPrototype(this);
        };

        mediaCodecDecoder *clone() override
        {
            return new mediaCodecDecoder();
        };

        bool is_supported(const Stream_meta &meta, uint64_t flags, int maxSize) override
        {
            if (flags & DECFLAG_HW)
                return checkSupport(meta, flags, maxSize);
            return false;
        };

        bool is_drmSupport(const DrmInfo *drmInfo) override {
            if(drmInfo == nullptr){
                return false;
            }

            bool drmSupport = drmInfo->format == "urn:uuid:edef8ba9-79d6-4ace-a3c8-27dcd51d21ed"
                               && DrmHandlerPrototype::isSupport(drmInfo);
            return drmSupport;
        }

        static mediaCodecDecoder se;

    private:
        int width{0};
        int height{0};
        int channel_count{0};
        int sample_rate{0};
        int format{0};

        int codecType = CODEC_VIDEO;
        std::string mMime{};

        MediaCodec_Decoder *mDecoder{nullptr};

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

        Stream_meta mMeta{};
        void* mVideoOutObser = nullptr;
        int naluLengthSize = 0;
        bool isADTS = false;

        WideVineDrmHandler* mDrmHandler = nullptr;

    };
}

#endif // QU_ANDROID_H264_DECODER_HH
