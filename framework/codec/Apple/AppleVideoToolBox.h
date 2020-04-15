#ifndef AFVTB_DECODER_H
#define AFVTB_DECODER_H

#include "codec/ActiveDecoder.h"
#include <map>
#include <VideoToolbox/VideoToolbox.h>
#include <CoreMedia/CoreMedia.h>
#include <queue>
#include <base/media/PBAFFrame.h>
#include <codec/utils_ios.h>
#include <utils/AFMediaType.h>
#include <utils/mediaTypeInternal.h>
#include <utils/bitStreamParser.h>
#include "../codecPrototype.h"

namespace Cicada{

    class CICADA_CPLUS_EXTERN AFVTBDecoder : public ActiveDecoder, private codecPrototype, private IOSNotificationObserver {
    public:
        AFVTBDecoder();

        ~AFVTBDecoder() override;

        static bool is_supported(enum AFCodecID codec);

        void setEOF() override
        {
        }

    private:
        int init_decoder(const Stream_meta *meta, void *wnd, uint64_t flags) override;

        void close_decoder() override;

        int enqueue_decoder(std::unique_ptr<IAFPacket> &pPacket) override;

        int dequeue_decoder(std::unique_ptr<IAFFrame> &pFrame) override;

        void flush_decoder() override;

        int get_decoder_recover_size() override;

        void flushReorderQueue();

    private:
        explicit AFVTBDecoder(int dummy)
        {
            addPrototype(this);
            mIsDummy = true;
        };

        AFVTBDecoder *clone() override
        {
            return new AFVTBDecoder();
        };

        bool is_supported(enum AFCodecID codec, uint64_t flags, int maxSize) override
        {
            if (!(flags & DECFLAG_HW))
                return false;
            return is_supported(codec);
        };
        static AFVTBDecoder se;

    private:

        int init_decoder_internal();

        void push_to_recovery_queue(std::unique_ptr<IAFPacket> pPacket);

        void gen_recovering_queue();

        int enqueue_decoder_internal(std::unique_ptr<IAFPacket> &pPacket);

        int process_extra_data(IAFPacket *pPacket);

        int createDecompressionSession(uint8_t *pData, int size, int width, int height);

        int createVideoFormatDesc(const uint8_t *pData, int size, int width, int height, CFDictionaryRef &decoder_spec,
                                  CMFormatDescriptionRef &cm_fmt_desc);

        static void decompressionOutputCallback(void *CM_NULLABLE decompressionOutputRefCon,
                                                void *CM_NULLABLE sourceFrameRefCon,
                                                OSStatus status,
                                                VTDecodeInfoFlags infoFlags,
                                                CM_NULLABLE CVImageBufferRef imageBuffer,
                                                CMTime presentationTimeStamp,
                                                CMTime presentationDuration);

        void onDecoded(IAFPacket *packet, std::unique_ptr<PBAFFrame> frame, OSStatus status);


        void AppWillResignActive() override;

        void AppDidBecomeActive() override;

#if 0
        void outputByPoc(VideoFrame *CM_NULLABLE beCachedFrame, int poc);
#endif
    private:
        CM_NULLABLE VTDecompressionSessionRef mVTDecompressSessionRef{nullptr};
        CM_NULLABLE CMVideoFormatDescriptionRef mVideoFormatDesRef{nullptr};
        CM_NULLABLE CFDictionaryRef mDecoder_spec{nullptr};
        const static int MAX_POC_ERROR = 3;
        CMVideoCodecType mVideoCodecType{0};
        std::map<int64_t, std::unique_ptr<PBAFFrame>> mReorderFrameMap;
        int mInputCount{0};
        bool mThrowPacket{false};
        std::mutex mActiveStatusMutex;
        std::atomic_bool mActive{true};
        AFPixelFormat mVTOutFmt = AF_PIX_FMT_NONE;
        std::unique_ptr<streamMeta> mPInMeta{nullptr};
        std::queue<std::unique_ptr<IAFPacket>> mRecoveryQueue{};
        std::queue<std::unique_ptr<IAFPacket>> mRecoveringQueue{};
        std::unique_ptr<bitStreamParser> mParser{nullptr};
        uint8_t mPocDelta = 2;
        std::atomic_int64_t mOutputPoc {0};
        bool mBUsePoc = false;
        std::queue<std::unique_ptr<IAFFrame>> mReorderedQueue{};
        std::mutex mReorderMutex{};

        bool mResignActive{false};
        bool mIsDummy = false;
        int mPocErrorCount{0};
    };

}

#endif
