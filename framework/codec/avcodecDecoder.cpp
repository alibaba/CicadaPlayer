//
// Created by moqi on 2018/8/10.
//
#define LOG_TAG "avcodecDecoder"

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
};

#include <utils/AFUtils.h>
#include <cstring>
#include <cstdlib>
#include <utils/frame_work_log.h>
#include <utils/mediaFrame.h>
#include <utils/ffmpeg_utils.h>
#include <cassert>
#include <deque>
#include "avcodecDecoder.h"
#include "base/media/AVAFPacket.h"
#include <utils/errors/framework_error.h>

#define  MAX_INPUT_SIZE 4

using namespace std;

namespace Cicada {
    avcodecDecoder avcodecDecoder::se(0);
#ifdef ENABLE_HWDECODER
    int init_hw_device(void *arg, enum AVHWDeviceType type, void *device)
    {
        avcodecVideoDecoder::decoder_handle_v *pHandle = (avcodecVideoDecoder::decoder_handle_v *) arg;
        int ret = -1;

        switch (type) {
#ifdef WIN32
#if (_MSC_VER >= 1700) && !defined(_USING_V110_SDK71_)

            case AV_HWDEVICE_TYPE_D3D11VA: {
                AVD3D11VADeviceContext *device_hwctx = (AVD3D11VADeviceContext *) device;

                if (pHandle->hwDeviceType_set == CICADA_HWDEVICE_TYPE_D3D11VA) {
                    CicadaD3D11VADeviceContext *pD3d11va = pHandle->phwDeviceCont_set;
                    device_hwctx->video_context = pD3d11va->video_context;
                    device_hwctx->video_device = pD3d11va->video_device;
                    device_hwctx->device = pD3d11va->device;
                    device_hwctx->device_context = pD3d11va->device_context;
                    ret = 0;
                }

                break;
            }

#endif
#endif

            default:
                ret = -1;
                break;
        }

        return ret;
    }
#endif

    void avcodecDecoder::close_decoder()
    {
        if (mPDecoder == nullptr) {
            return;
        }

#ifdef ENABLE_HWDECODER

        if (mPDecoder->pHWHandle) {
            release_hw_content(mPDecoder->pHWHandle);
        }

#endif

        if (mPDecoder->codecCont != nullptr) {
            avcodec_close(mPDecoder->codecCont);
#ifdef ENABLE_HWDECODER

            if (mPDecoder->hwaccel_uninit) {
                mPDecoder->hwaccel_uninit(mPDecoder->vc);
            }

#endif
            avcodec_free_context(&mPDecoder->codecCont);
            mPDecoder->codecCont = nullptr;
        }

        mPDecoder->codec = nullptr;
        av_frame_free(&mPDecoder->avFrame);
        delete mPDecoder;
        mPDecoder = nullptr;
    }

    int avcodecDecoder::init_decoder(const Stream_meta *meta, void *wnd, uint64_t flags)
    {
        auto codecId = (enum AVCodecID) CodecID2AVCodecID(meta->codec);
        mPDecoder->codec = avcodec_find_decoder(codecId);
        bool isAudio = meta->channels > 0;

        if (mPDecoder->codec == nullptr) {
            return gen_framework_errno(error_class_codec, isAudio ? codec_error_audio_not_support : codec_error_video_not_support);
        }

        mPDecoder->codecCont = avcodec_alloc_context3((const AVCodec *) mPDecoder->codec);

        if (mPDecoder->codecCont == nullptr) {
            AF_LOGE("init_decoder error");
            return gen_framework_errno(error_class_codec, isAudio ? codec_error_audio_not_support : codec_error_video_not_support);
        }

        // TODO: not set extradata when XXvc

        if (AF_CODEC_ID_PCM_S16LE == meta->codec) {
            mPDecoder->codecCont->channels = meta->channels;
            mPDecoder->codecCont->sample_rate = meta->samplerate;
        }

        if (meta->extradata != nullptr && meta->extradata_size > 0) {
            mPDecoder->codecCont->extradata = (uint8_t *) av_mallocz(meta->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
            memcpy(mPDecoder->codecCont->extradata, meta->extradata, meta->extradata_size);
            mPDecoder->codecCont->extradata_size = meta->extradata_size;
        }

        mPDecoder->flags = DECFLAG_SW;
#ifdef ENABLE_HWDECODER

        if (flags & DECFLAG_HW) {
#ifdef WIN32
            mPDecoder->pHWHandle = create_hw_content("dxva2");
#endif

            if (mPDecoder->pHWHandle) {
                hw_content_set_device_init_cb(mPDecoder->pHWHandle, init_hw_device, mPDecoder);
                mPDecoder->vc->opaque = mPDecoder->pHWHandle;

                if (hw_decoder_init(mPDecoder->vc, wnd) < 0) {
                    release_hw_content(mPDecoder->pHWHandle);
                    mPDecoder->vc->opaque = nullptr;
                    mPDecoder->pHWHandle = nullptr;
                } else {
                    mPDecoder->flags = DECFLAG_HW;
                    mPDecoder->vc->get_format = get_hw_format;

                    if ((flags & DECFLAG_DIRECT) == 0) {
                        mPDecoder->hwaccel_retrieve_data = hwaccel_retrieve_data;
                    } else {
                        mPDecoder->flags |= DECFLAG_DIRECT;
                    }
                }
            }
        }

#endif
        av_opt_set_int(mPDecoder->codecCont, "refcounted_frames", 1, 0);
        int threadcount = (AFGetCpuCount() > 0 ? AFGetCpuCount() + 1 : 0);

        if ((flags & DECFLAG_OUTPUT_FRAME_ASAP)
                && ((0 == threadcount) || (threadcount > 2))) {
            // set too much thread need more video buffer in ffmpeg
            threadcount = 2;
        }

        AF_LOGI("set decoder thread as :%d\n", threadcount);
        mPDecoder->codecCont->thread_count = threadcount;

        if (avcodec_open2(mPDecoder->codecCont, mPDecoder->codec, nullptr) < 0) {
            AF_LOGE("could not open codec\n");
            avcodec_free_context(&mPDecoder->codecCont);
            return -1;
        }

        mPDecoder->avFrame = av_frame_alloc();
        mPDecoder->vInfo.height = mPDecoder->codecCont->height;
        mPDecoder->vInfo.width = mPDecoder->codecCont->width;
        mPDecoder->vInfo.pix_fmt = mPDecoder->codecCont->pix_fmt;
        return 0;
    }


    avcodecDecoder::avcodecDecoder() : ActiveDecoder()
    {
        mName = "VD.avcodec";
        mPDecoder = new decoder_handle_v();
        memset(mPDecoder, 0, sizeof(decoder_handle_v));
//        mPDecoderder->dstFormat = AV_PIX_FMT_NONE;
#ifdef ENABLE_HWDECODER
        mPDecoder->hwDeviceType_set = CICADA_HWDEVICE_TYPE_UNKNOWN;
#endif
        avcodec_register_all();
    }

    avcodecDecoder::~avcodecDecoder()
    {
        close();
    }

    bool avcodecDecoder::is_supported(enum AFCodecID codec)
    {
//        return codec == AF_CODEC_ID_H264
//               || codec == AF_CODEC_ID_MPEG4
//               || codec == AF_CODEC_ID_HEVC
//               || codec == AF_CODEC_ID_AAC
//               || codec == AF_CODEC_ID_MP1
//               || codec == AF_CODEC_ID_MP2
//               || codec == AF_CODEC_ID_MP3
//               || codec == AF_CODEC_ID_PCM_S16LE;
        if (avcodec_find_decoder(CodecID2AVCodecID(codec))) {
            return true;
        }

        return false;
    }

    void avcodecDecoder::flush_decoder()
    {
        avcodec_flush_buffers(mPDecoder->codecCont);
    }

    int avcodecDecoder::dequeue_decoder(unique_ptr<IAFFrame> &pFrame)
    {
        int ret = avcodec_receive_frame(mPDecoder->codecCont, mPDecoder->avFrame);

        if (ret < 0) {
            if (ret == AVERROR_EOF) {
                return STATUS_EOS;
            }

            return ret;
        }

        if (mPDecoder->avFrame->decode_error_flags || mPDecoder->avFrame->flags) {
            AF_LOGW("get a error frame\n");
            return -EAGAIN;
        }

#ifdef ENABLE_HWDECODER

        if (mPDecoder->hwaccel_retrieve_data) {
            mPDecoder->hwaccel_retrieve_data(mPDecoder->vc, mPDecoder->picture);
        }

#endif
        pFrame = unique_ptr<IAFFrame>(new AVAFFrame(mPDecoder->avFrame));
        return ret;
    };

    int avcodecDecoder::enqueue_decoder(unique_ptr<IAFPacket> &pPacket)
    {
        int ret;
        AVPacket *pkt = nullptr;

        if (pPacket) {
            auto *avAFPacket = dynamic_cast<AVAFPacket *>(pPacket.get());
            assert(avAFPacket);

            if (avAFPacket == nullptr) {
                // TODO: tobe impl
            } else {
                pkt = avAFPacket->ToAVPacket();
                pkt->pts = pPacket->getInfo().pts;
                pkt->dts = pPacket->getInfo().dts;
                assert(pkt != nullptr);
            }
        }

        if (pkt == nullptr) {
            AF_LOGD("send null to decoder\n");
        }

        ret = avcodec_send_packet(mPDecoder->codecCont, pkt);

        if (0 == ret) {
            pPacket = nullptr;
        } else if (ret == AVERROR(EAGAIN)) {
        } else if (ret == AVERROR_EOF) {
            AF_LOGD("Decode EOF\n");
            ret = 0;
        } else {
            AF_LOGE("Error while decoding frame %d :%s\n", ret,  getErrorString(ret));
        }

        return ret;
    }


}
