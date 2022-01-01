#define LOG_TAG "mediaCodecDecoder"

#include "mediaCodecDecoder.h"
#include <utils/frame_work_log.h>
#include <utils/timer.h>
#include <utils/Android/systemUtils.h>
#include <cassert>
#include <map>
#include <utils/ffmpeg_utils.h>
#include <drm/DrmHandlerPrototype.h>
#include <drm/WideVineDrmHandler.h>

extern "C" {
#include <libavutil/intreadwrite.h>
#include <utils/errors/framework_error.h>
}

#define  MAX_INPUT_SIZE 4
using namespace std;
namespace Cicada {

    typedef struct blackModelDevice {
        AFCodecID codec;
        string model;
    } blackModelDevice;
    blackModelDevice blackList[] = {
            {AF_CODEC_ID_H264, "2014501"},
            {AF_CODEC_ID_HEVC, "OPPO R9tm"},
            {AF_CODEC_ID_HEVC, "OPPO A59s"},
    };

    mediaCodecDecoder mediaCodecDecoder::se(0);

    mediaCodecDecoder::mediaCodecDecoder() {
        AF_LOGD("android decoder use jni");
        mName = "VD.mediaCodec";
        mFlags |= DECFLAG_HW;
        mDecoder = new MediaCodec_Decoder();
    }

    mediaCodecDecoder::~mediaCodecDecoder() {
        delete mDecoder;
    }

    bool mediaCodecDecoder::checkSupport(const Stream_meta &meta, uint64_t flags, int maxSize) {
        AFCodecID codec = meta.codec;
        if (codec != AF_CODEC_ID_H264 && codec != AF_CODEC_ID_HEVC
            && codec != AF_CODEC_ID_AAC) {
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
        string model = get_android_property("ro.product.model");
        for (auto device : blackList) {
            if (device.codec == codec && device.model == model) {
                AF_LOGI("device %d@%s is in black list\n", device.codec, device.model.c_str());
                return false;
            }
        }

        return true;
    }

    int mediaCodecDecoder::init_decoder(const Stream_meta *meta, void *voutObsr, uint64_t flags,
                                        const DrmInfo *drmInfo) {
        if (meta->pixel_fmt == AF_PIX_FMT_YUV422P || meta->pixel_fmt == AF_PIX_FMT_YUVJ422P) {
            return -ENOSPC;
        }

        if (!checkSupport(*meta, flags, max(meta->height, meta->width)) ||
                (drmInfo != nullptr && !is_drmSupport(drmInfo))) {
            return -ENOSPC;
        }

        if (flags & DECFLAG_DIRECT) {
            mFlags |= DECFLAG_OUT;
        }

        if (meta->codec == AF_CODEC_ID_H264) {
            codecType = CODEC_VIDEO;
            mMime = "video/avc";
        } else if (meta->codec == AF_CODEC_ID_HEVC) {
            codecType = CODEC_VIDEO;
            mMime = "video/hevc";
        } else if (meta->codec == AF_CODEC_ID_AAC) {
            codecType = CODEC_AUDIO;
            mMime = "audio/mp4a-latm";
        } else {
            AF_LOGE("codec is %d, not support", meta->codec);
            return -ENOSPC;
        }

        mMeta = *meta;
        mVideoOutObser = voutObsr;

        lock_guard<recursive_mutex> func_entry_lock(mFuncEntryMutex);
        setCSD(meta);

        if (drmInfo != nullptr) {
            if (mRequireDrmHandlerCallback != nullptr) {
                mDrmHandler = dynamic_cast<WideVineDrmHandler *>(mRequireDrmHandlerCallback(
                        *drmInfo));
                assert(mDrmHandler != nullptr);
            }

            int ret = initDrmHandler();
            if (ret == -EAGAIN) {
                return 0;
            } else if (ret < 0) {
                return ret;
            }
        }

        return configDecoder();
    }

    int mediaCodecDecoder::setCSD(const Stream_meta *meta) {
        if (meta->codec == AF_CODEC_ID_HEVC) {

            if (meta->extradata == nullptr || meta->extradata_size == 0) {
                return -1;
            }

            uint8_t *vps_data = nullptr;
            uint8_t *sps_data = nullptr;
            uint8_t *pps_data = nullptr;
            int vps_data_size = 0;
            int sps_data_size = 0;
            int pps_data_size = 0;

            int ret = parse_h265_extraData(CodecID2AVCodecID(AF_CODEC_ID_HEVC),meta->extradata, meta->extradata_size,
                                           &vps_data,&vps_data_size,
                                           &sps_data,&sps_data_size,
                                           &pps_data,&pps_data_size,
                                           &naluLengthSize);
            if (ret >= 0) {

                std::list<CodecSpecificData> csdList{};
                CodecSpecificData csd0{};

                const int data_size =
                        vps_data_size + sps_data_size + pps_data_size;
                char data[data_size];

                memcpy(data, vps_data, vps_data_size);
                memcpy(data + vps_data_size, sps_data, sps_data_size);
                memcpy(data + vps_data_size + sps_data_size, pps_data, pps_data_size);

                csd0.setScd("csd-0", data, data_size);
                csdList.push_back(csd0);
                mDecoder->setCodecSpecificData(csdList);

                csdList.clear();
            }

            return ret;
        } else if (meta->codec == AF_CODEC_ID_H264) {

            if (meta->extradata == nullptr || meta->extradata_size == 0) {
                return -1;
            }

            uint8_t *sps_data = nullptr;
            uint8_t *pps_data = nullptr;
            int sps_data_size = 0;
            int pps_data_size = 0;

            int ret = parse_h264_extraData(CodecID2AVCodecID(AF_CODEC_ID_H264),meta->extradata, meta->extradata_size,
                                           &sps_data,&sps_data_size,
                                           &pps_data,&pps_data_size,
                                           &naluLengthSize);
            if (ret >= 0) {

                std::list<CodecSpecificData> csdList{};
                CodecSpecificData csd0{};
                csd0.setScd("csd-0", sps_data, sps_data_size);
                csdList.push_back(csd0);
                CodecSpecificData csd1{};
                csd1.setScd("csd-1", pps_data, pps_data_size);
                csdList.push_back(csd1);
                mDecoder->setCodecSpecificData(csdList);

                csdList.clear();
            }

            return ret;
        } else if (meta->codec == AF_CODEC_ID_AAC) {
            if (meta->extradata == nullptr || meta->extradata_size == 0) {
                isADTS = true;
                //ADTS, has no extra data . MediaCodec MUST set csd when decode aac

                int samplingFreq[] = {
                        96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050,
                        16000, 12000, 11025, 8000
                };

                // Search the Sampling Frequencies
                int sampleIndex = -1;
                for (int i = 0; i < 12; ++i) {
                    if (samplingFreq[i] == mMeta.samplerate) {
                        sampleIndex = i;
                        break;
                    }
                }
                if (sampleIndex < 0) {
                    return -1;
                }

                const size_t kCsdLength = 2;
                char csd[kCsdLength];
                csd[0] = (meta->profile + 1) << 3 | sampleIndex >> 1;
                csd[1] = (sampleIndex & 0x01) << 7 | meta->channels << 3;

                std::list<CodecSpecificData> csdList{};
                CodecSpecificData csd0{};
                csd0.setScd("csd-0", csd, kCsdLength);
                csdList.push_back(csd0);
                mDecoder->setCodecSpecificData(csdList);

                csdList.clear();
            } else {
                isADTS = false;
                std::list<CodecSpecificData> csdList{};
                CodecSpecificData csd0{};
                csd0.setScd("csd-0", meta->extradata, meta->extradata_size);
                csdList.push_back(csd0);
                mDecoder->setCodecSpecificData(csdList);

                csdList.clear();
            }
            return 0;
        } else {
            return -1;
        }
    }

    void mediaCodecDecoder::flush_decoder() {
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

    void mediaCodecDecoder::close_decoder() {
        lock_guard<recursive_mutex> func_entry_lock(mFuncEntryMutex);

        // stop decoder.
        // must before destructor producer because inner thread will use surface.
        if (mbInit) {
            mFlushState = 0;
            mDecoder->stop();
            releaseDecoder();
            mbInit = false;
        }

        mInputFrameCount = 0;
    }

    void mediaCodecDecoder::releaseDecoder() {
        if (mDecoder != nullptr) {
            mDecoder->release();
        }
    }

    int mediaCodecDecoder::enqueue_decoder(unique_ptr<IAFPacket> &pPacket) {

        if (!mbInit && mDrmHandler != nullptr) {
            int ret = initDrmHandler();
            if (ret == -EAGAIN) {
                return -EAGAIN;
            } else if (ret < 0) {
                return ret;
            } else if (ret == 0) {
                ret = configDecoder();

                if (ret < 0) {
                    return ret;
                }
            }
        }

        int index = mDecoder->dequeueInputBufferIndex(1000);

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
                AF_LOGD("queue eos codecType = %d\n", codecType);
            }

            if (mDrmHandler != nullptr) {
                IAFPacket::EncryptionInfo encryptionInfo{};
                if (pPacket != nullptr) {
                    pPacket->getEncryptionInfo(&encryptionInfo);
                }

                uint8_t *new_data = nullptr;
                int new_size = 0;
                mDrmHandler->convertData(naluLengthSize, &new_data, &new_size, data, size);

                if (new_data != nullptr) {
                    data = new_data;
                    size = new_size;
                }

                ret = mDecoder->queueSecureInputBuffer(index, data, static_cast<size_t>(size),
                                                       &encryptionInfo, pts,
                                                       false);

                if (new_data != nullptr) {
                    free(new_data);
                }

            } else {
                ret = mDecoder->queueInputBuffer(index, data, static_cast<size_t>(size), pts,
                                                 false);
            }

            if (ret < 0) {
                AF_LOGE(" mDecoder->queue_in error codecType = %d\n", codecType);
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

    int mediaCodecDecoder::dequeue_decoder(unique_ptr<IAFFrame> &pFrame) {
        int ret;
        int index;
        index = mDecoder->dequeueOutputBufferIndex(1000);

        if (index == MC_ERROR) {
            AF_LOGE("dequeue_out occur error. flush state %d", mFlushState);
            return MC_ERROR;
        } else if (index == MC_INFO_TRYAGAIN || index == MC_INFO_OUTPUT_BUFFERS_CHANGED) {
            return -EAGAIN;
        } else if (index == MC_INFO_OUTPUT_FORMAT_CHANGED) {
            mc_out out{};
            mDecoder->getOutput(index, &out, false);

            if (codecType == CODEC_VIDEO) {
                height = out.conf.video.height;

                if (out.conf.video.crop_bottom != MC_ERROR && out.conf.video.crop_top != MC_ERROR) {
                    height = out.conf.video.crop_bottom + 1 - out.conf.video.crop_top;
                }

                width = out.conf.video.width;

                if (out.conf.video.crop_right != MC_ERROR && out.conf.video.crop_left != MC_ERROR) {
                    width = out.conf.video.crop_right + 1 - out.conf.video.crop_left;
                }
            } else if (codecType == CODEC_AUDIO) {
                channel_count = out.conf.audio.channel_count;
                sample_rate = out.conf.audio.sample_rate;
                format = out.conf.audio.format;
            }

            return -EAGAIN;
        } else if (index >= 0) {
            mc_out out{};
            ret = mDecoder->getOutput(index, &out, codecType != CODEC_VIDEO);
            auto item = mDiscardPTSSet.find(out.buf.pts);

            if (item != mDiscardPTSSet.end()) {
                mDecoder->releaseOutputBuffer(index, false);
                mDiscardPTSSet.erase(item);
                return -EAGAIN;
            }

            if (out.b_eos) {
                return STATUS_EOS;
            }

            // AF_LOGD("mediacodec out pts %" PRId64, out.buf.pts);
            if (codecType == CODEC_VIDEO) {
                pFrame = unique_ptr<AFMediaCodecFrame>(
                        new AFMediaCodecFrame(IAFFrame::FrameTypeVideo, index,
                                              [this](int index, bool render) {
                                                  mDecoder->releaseOutputBuffer(index, render);
                                              }));
                pFrame->getInfo().video.width = width;
                pFrame->getInfo().video.height = height;
            } else if (codecType == CODEC_AUDIO) {

                assert(out.buf.p_ptr != nullptr);

                if (out.buf.p_ptr == nullptr) {
                    return -EAGAIN;
                }

                AFSampleFormat afFormat = AFSampleFormat::AF_SAMPLE_FMT_NONE;
                if (format < 0 || format == 2) {
                    afFormat = AF_SAMPLE_FMT_S16;
                } else if (format == 3) {
                    afFormat = AF_SAMPLE_FMT_U8;
                } else if (format == 4) {
                    afFormat = AF_SAMPLE_FMT_S32;
                }

                assert(afFormat != AFSampleFormat::AF_SAMPLE_FMT_NONE);

                IAFFrame::AFFrameInfo frameInfo{};
                frameInfo.audio.format = afFormat;
                frameInfo.audio.sample_rate = sample_rate;
                frameInfo.audio.channels = channel_count;

                uint8_t *data[1] = {nullptr};
                data[0] = const_cast<uint8_t *>(out.buf.p_ptr);
                int lineSize[1] = {0};
                lineSize[0] = out.buf.size;

                pFrame = unique_ptr<AVAFFrame>(
                        new AVAFFrame(frameInfo, (const uint8_t **) data, (const int *) lineSize, 1,
                                      IAFFrame::FrameTypeAudio));
                mDecoder->releaseOutputBuffer(index, false);

                pFrame->getInfo().audio.sample_rate = sample_rate;
                pFrame->getInfo().audio.channels = channel_count;
                pFrame->getInfo().audio.format = afFormat;
            }

            pFrame->getInfo().pts = out.buf.pts != -1 ? out.buf.pts : INT64_MIN;

            // TODO: get the timePosition form input packet
            pFrame->getInfo().timePosition = INT64_MIN;
            return 0;
        } else {
            AF_LOGE("unknown error %d\n", index);
            return index;
        }
    }

    int mediaCodecDecoder::configDecoder() {

        int ret = -1;
        if (codecType == CODEC_VIDEO) {
            ret = mDecoder->configureVideo(mMime, mMeta.width, mMeta.height, 0,
                                           static_cast<jobject>(mVideoOutObser));
        } else if (codecType == CODEC_AUDIO) {
            ret = mDecoder->configureAudio(mMime, mMeta.samplerate, mMeta.channels,isADTS);
        }

        if (ret >= 0) {
            ret = 0;
        } else {
            AF_LOGE("failed to config mDecoder rv %d", ret);
            releaseDecoder();
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

    int mediaCodecDecoder::initDrmHandler() {

        mDrmHandler->open();

        int state = mDrmHandler->getState();
        if (state == SESSION_STATE_OPENED) {
            bool insecure = mDrmHandler->isForceInsecureDecoder();
            mDecoder->setForceInsecureDecoder(insecure);

            char *sessionId = nullptr;
            int sessionSize = mDrmHandler->getSessionId(&sessionId);
            mDecoder->setDrmInfo("edef8ba9-79d6-4ace-a3c8-27dcd51d21ed", sessionId,
                                 sessionSize);
            return 0;
        } else if (state == SESSION_STATE_IDLE) {
            return -EAGAIN;
        } else if (state == SESSION_STATE_ERROR) {
            return mDrmHandler->getErrorCode();
        }
        return -EAGAIN;
    }
}
