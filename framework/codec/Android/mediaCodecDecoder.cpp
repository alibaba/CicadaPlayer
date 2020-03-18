#define LOG_TAG "mediaCodecDecoder"

#include "mediaCodecDecoder.h"
#include "mediaCodec.h"
#include <utils/frame_work_log.h>
#include <utils/errors/framework_error.h>
#include <utils/timer.h>
#include <utils/Android/systemUtils.h>
#include <cassert>

#define  MAX_INPUT_SIZE 4
using namespace std;
namespace Cicada {
    mediaCodecDecoder mediaCodecDecoder::se(0);

    mediaCodecDecoder::mediaCodecDecoder()
    {
        AF_LOGD("android decoder use jni");
        mFlags |= DECFLAG_HW;
        mDecoder = new MediaCodec_JNI();
    }

    mediaCodecDecoder::~mediaCodecDecoder()
    {
        lock_guard<recursive_mutex> func_entry_lock(mFuncEntryMutex);
        delete mDecoder;
    }

    bool mediaCodecDecoder::checkSupport(AFCodecID codec, uint64_t flags, int maxSize)
    {
        if (codec != AF_CODEC_ID_H264 && codec != AF_CODEC_ID_HEVC) {
            return false;
        }

        string version = get_android_property("ro.build.version.sdk");

        if (atoi(version.c_str()) < 16) {
            return false;
        }

        if (atoi(version.c_str()) < 21) {
            if (flags & DECFLAG_ADAPTIVE || codec == AF_CODEC_ID_HEVC || maxSize > 1920) {
                return false;
            }
        }

        return true;
    }

    int mediaCodecDecoder::init_decoder(const Stream_meta *meta, void *voutObsr, uint64_t flags)
    {
        if (meta->pixel_fmt == AF_PIX_FMT_YUV422P || meta->pixel_fmt == AF_PIX_FMT_YUVJ422P) {
            return -ENOSPC;
        }

        if (!checkSupport(meta->codec, flags, max(meta->height, meta->width))) {
            return -ENOSPC;
        }

        if (flags & DECFLAG_DIRECT) {
            mFlags |= DECFLAG_OUT;
        }

        const char *mime;

        if (meta->codec == AF_CODEC_ID_H264) {
            mime = "video/avc";
        } else if (meta->codec == AF_CODEC_ID_HEVC) {
            mime = "video/hevc";
        } else {
            AF_LOGE("codec is %d, not support", meta->codec);
            return -ENOSPC;
        }

        lock_guard<recursive_mutex> func_entry_lock(mFuncEntryMutex);
        int ret;
        ret = mDecoder->init(mime, CATEGORY_VIDEO, static_cast<jobject>(voutObsr));

        if (ret == MC_ERROR || ret < 0) {
            AF_LOGE("failed to init mDecoder, ret %d", ret);
            mDecoder->unInit();
            return gen_framework_errno(error_class_codec, codec_error_video_device_error);
        }

        mc_args args{};
        args.video.width = meta->width;
        args.video.height = meta->height;
        args.video.angle = 0;
        ret = mDecoder->configure(0, args);

        if (ret >= 0) {
            ret = 0;
        } else {
            AF_LOGE("failed to config mDecoder rv %d", ret);
            mDecoder->unInit();
            ret = gen_framework_errno(error_class_codec, codec_error_video_device_error);
        }

        if (ret == 0) {
            if (mDecoder->start() == MC_ERROR) {
                AF_LOGE("mediacodec start failed.");
                return gen_framework_errno(error_class_codec, codec_error_video_device_error);
            }

            mbInit = true;
            mFlushState = 1;
        }

        return ret;
    }

    void mediaCodecDecoder::flush_decoder()
    {
        lock_guard<recursive_mutex> func_entry_lock(mFuncEntryMutex);
        mOutputFrameCount = 0;

        if (!mbInit) {
            return;
        }

        if (mInputFrameCount <= 0) {
            return;
        }

        {
            std::lock_guard<std::mutex> l(mFlushInterruptMuex);
            mFlushState = 1;
            int ret = mDecoder->flush();
            AF_LOGI("clearCache. ret %d, flush state %d", ret, mFlushState);
        }

        mDiscardPTSSet.clear();
        mInputFrameCount = 0;
    }

    void mediaCodecDecoder::close_decoder()
    {
        lock_guard<recursive_mutex> func_entry_lock(mFuncEntryMutex);

        // stop decoder.
        // must before destructor producer because inner thread will use surface.
        if (mbInit) {
            mFlushState = 0;
            mDecoder->stop();
            mDecoder->unInit();
            mbInit = false;
        }

        mInputFrameCount = 0;
    }


    int mediaCodecDecoder::enqueue_decoder(unique_ptr<IAFPacket> &pPacket)
    {
        int index = mDecoder->dequeue_in(1000);

        if (index == MC_ERROR) {
            AF_LOGE("dequeue_in error.");
            // TODO: value
            return -ENOSPC;
        } else if (index == MC_INFO_TRYAGAIN) {
            return -EAGAIN;
        }

        int ret = 0;

        if (index >= 0) {
//            if (pPacket != nullptr)
//                AF_LOGD("mediacodec in pts %" PRId64 " size %d", pPacket->getInfo().pts, pPacket->getSize());
            uint8_t *data = nullptr;
            int size = 0;
            int64_t pts = 0;

            if (pPacket != nullptr) {
                data = pPacket->getData();
                size = static_cast<int>(pPacket->getSize());
                pts = pPacket->getInfo().pts;

                if (pPacket->getDiscard()) {
                    mDiscardPTSSet.insert(pts);
                }
            } else {
                AF_LOGD("queue eos\n");
            }

            ret = mDecoder->queue_in(index, data, static_cast<size_t>(size), pts, false);

            if (ret < 0) {
                AF_LOGE(" mDecoder->queue_in error \n");
            }

            mInputFrameCount++;
        }

        if (mFlushState == 1) {
            std::lock_guard<std::mutex> l(mFlushInterruptMuex);

            if (pPacket != nullptr) {
                AF_LOGI("send Frame mFlushState = 2. pts %"
                        PRId64, pPacket->getInfo().pts);
            }

            mFlushState = 2;
        }

        if (ret == 0) {
            return 0;
        } else {
            AF_LOGE("queue_in error. ret %d", ret);
            return -ENOSPC;
        }

        return ret;
    }

    int mediaCodecDecoder::dequeue_decoder(unique_ptr<IAFFrame> &pFrame)
    {
        int ret;
        int index;
        index = mDecoder->dequeue_out(1000);

        if (index == MC_ERROR) {
            AF_LOGE("dequeue_out occur error. flush state %d", mFlushState);
            return MC_ERROR;
        } else if (index == MC_INFO_TRYAGAIN || index == MC_INFO_OUTPUT_BUFFERS_CHANGED) {
            return -EAGAIN;
        } else if (index == MC_INFO_OUTPUT_FORMAT_CHANGED) {
            mc_out out{};
            mDecoder->get_out(index, &out, false);
            mVideoInfo.height = out.conf.video.height;

            if (out.conf.video.crop_bottom != MC_ERROR && out.conf.video.crop_top != MC_ERROR) {
                mVideoInfo.height = out.conf.video.crop_bottom + 1 - out.conf.video.crop_top;
            }

            mVideoInfo.width = out.conf.video.width;

            if (out.conf.video.crop_right != MC_ERROR && out.conf.video.crop_left != MC_ERROR) {
                mVideoInfo.width = out.conf.video.crop_right + 1 - out.conf.video.crop_left;
            }

            return -EAGAIN;
        } else if (index >= 0) {
            mc_out out{};
            ret = mDecoder->get_out(index, &out, false);
            auto item = mDiscardPTSSet.find(out.buf.pts);

            if (item != mDiscardPTSSet.end()) {
                mDecoder->release_out(index, false);
                mDiscardPTSSet.erase(item);
                return -EAGAIN;
            }

            pFrame = unique_ptr<AFMediaCodecFrame>(new AFMediaCodecFrame(IAFFrame::FrameTypeVideo, index,
            [this](int index, bool render) {
                mDecoder->release_out(index, render);
            }));
            // AF_LOGD("mediacodec out pts %" PRId64, out.buf.pts);
            pFrame->getInfo().video = mVideoInfo;
            pFrame->getInfo().pts = out.buf.pts != -1 ? out.buf.pts : INT64_MIN;

            if (out.b_eos) {
                return STATUS_EOS;
            }

            return 0;
        } else {
            AF_LOGE("unknown error %d\n", index);
            return index;
        }
    }
}
