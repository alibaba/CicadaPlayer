//
// Created by moqi on 2018/8/10.
//

#ifndef FRAMEWORK_AVCODECDECODER_H
#define FRAMEWORK_AVCODECDECODER_H

#include "codec/ActiveDecoder.h"

#include <mutex>
#include <codec/IDecoder.h>
#include "base/media/AVAFPacket.h"
#include "codecPrototype.h"

//#define ENABLE_HWDECODER

typedef struct cicada_decoder_handle_v_t cicada_decoder_handle_v;

namespace Cicada{
    class CICADA_CPLUS_EXTERN avcodecDecoder : public ActiveDecoder, private codecPrototype {
    private:
        struct decoder_handle_v {
            AVCodecContext *codecCont;
            AVCodec *codec;
            AVFrame *avFrame;
            video_info vInfo;
//            struct SwsContext *img_convert_ctx;
//            int swscale_panding;
//            AVFrame *tmp_picture;
//            enum AVPixelFormat dstFormat;
#ifdef ENABLE_HWDECODER
            void *hwaccel_ctx;

            void (*hwaccel_uninit)(AVCodecContext *s);

            int (*hwaccel_get_buffer)(AVCodecContext *s, AVFrame *frame, int flags);

            int (*hwaccel_retrieve_data)(AVCodecContext *s, AVFrame *frame);

            enum AVPixelFormat hwaccel_pix_fmt;
            enum AVPixelFormat hwaccel_retrieved_pix_fmt;
            AVFrame *tmp_frame;
            hw_content *pHWHandle;
            void *phwDeviceCont_set;
            CICADAHWDeviceType hwDeviceType_set;
#endif
            int flags;
        };
    public:
        avcodecDecoder();

        ~avcodecDecoder() override;

        static bool is_supported(enum AFCodecID codec);

        void setEOF() override
        {
        }

    private:
        explicit avcodecDecoder(int dummy)
        {
            addPrototype(this);
        };

        avcodecDecoder *clone() override
        {
            return new avcodecDecoder();
        };

        bool is_supported(AFCodecID codec, uint64_t flags, int maxSize) override
        {
            if (flags & DECFLAG_SW)
                return is_supported(codec);
            return false;
        };
        static avcodecDecoder se;

    private:

        int enqueue_decoder(std::unique_ptr<IAFPacket> &pPacket) override;

        int dequeue_decoder(std::unique_ptr<IAFFrame> &pFrame) override;

        int init_decoder(const Stream_meta *meta, void *wnd, uint64_t flags) override;

        void close_decoder() override;

        void flush_decoder() override;

        int get_decoder_recover_size() override
        {
            return 0;
        };

    private:
        decoder_handle_v *mPDecoder = nullptr;
    };
}


#endif //FRAMEWORK_AVCODECDECODER_H
