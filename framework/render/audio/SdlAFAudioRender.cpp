//
// Created by lifujun on 2019/7/23.
//

#define LOG_TAG "SdlAFAudioRender"

#include <utils/ffmpeg_utils.h>
#include <base/media/AVAFPacket.h>
#include "SdlAFAudioRender.h"

namespace Cicada {

    SdlAFAudioRender::SdlAFAudioRender()
    {
    }

    SdlAFAudioRender::~SdlAFAudioRender()
    {
        if (mDevID > 0) {
            SDL_CloseAudioDevice(mDevID);
            mDevID = 0;
        }
        SDL_QuitSubSystem(SDL_INIT_AUDIO);

        if (pcmBuffer != nullptr) {
            free(pcmBuffer);
        }

        while (!mAudioFrames.empty()) {
            mAudioFrames.pop();
        }
    }

    int SdlAFAudioRender::init(const IAFFrame::audioInfo *info)
    {
        if (mInited) {
            //   AF_LOGE("SdlAFVideoRender has initialize!");
            return 1;
        }

        mInfo = *info;
        mInited = true;
        int initRet = SDL_Init(SDL_INIT_AUDIO);

        if (initRet < 0) {
            AF_LOGE("SdlAFAudioRender could not initialize! Error: %s\n", SDL_GetError());
            return initRet;
        }

        return 0;
    }

    int SdlAFAudioRender::renderFrame(std::unique_ptr<IAFFrame> &frame, int timeOut)
    {
        // AF_LOGD("SDLAudioCallback , add one frame ....");
        if (mDevID == 0) {
            int nb_samples = frame->getInfo().audio.nb_samples;
            int channels = frame->getInfo().audio.channels;
            int sample_rate = frame->getInfo().audio.sample_rate;
            int format = frame->getInfo().audio.format;
            mSpec.freq = sample_rate;

            if (format == AF_SAMPLE_FMT_S16 || format == AF_SAMPLE_FMT_S16P) {
                mSpec.format = AUDIO_S16SYS;
            } else if (format == AF_SAMPLE_FMT_FLT || format == AF_SAMPLE_FMT_FLTP) {
                mSpec.format = AUDIO_F32SYS;
            }

            mSpec.channels = channels;
            mSpec.silence = 0;
            mSpec.samples = nb_samples;
            mSpec.userdata = this;
            mSpec.callback = nullptr;
            const AVFrame *avFrame = getAVFrame(frame.get());
            bufferSize = getPCMDataLen(frame->getInfo().audio.channels, (enum AVSampleFormat) frame->getInfo().audio.format,
                                       frame->getInfo().audio.nb_samples);
            pcmBuffer = static_cast<uint8_t *>(malloc(bufferSize));
            mDevID = SDL_OpenAudioDevice(NULL, false, &mSpec, nullptr, SDL_AUDIO_ALLOW_ANY_CHANGE);
            if (mDevID == 0) {
                AF_LOGE("SdlAFAudioRender could not openAudio! Error: %s\n", SDL_GetError());
                return -1;
            }
            SDL_PauseAudioDevice(mDevID, 0); // start play audio
        }
        if (frame->getInfo().duration < 0 && frame->getInfo().audio.sample_rate > 0) {
            frame->getInfo().duration = frame->getInfo().audio.nb_samples * 100000 / frame->getInfo().audio.sample_rate;
        }

        //  mAudioFrames.push(std::move(frame));
        if (getQueDuration() > 500 * 1000) {
            return -EAGAIN;
        }

        if (mFiler != nullptr) {
            int ret;
            unique_ptr<IAFFrame> filter_frame{};
            ret = mFiler->pull(filter_frame, 0);

            if (ret != -EAGAIN) {
                //          AF_LOGD("nb samples is %d\n", filter_frame->getInfo().audio.nb_samples);
                int pcmDataLength = getPCMDataLen(filter_frame->getInfo().audio.channels, 
                                                    (enum AVSampleFormat) filter_frame->getInfo().audio.format,
                                                    filter_frame->getInfo().audio.nb_samples);
                if (bufferSize < pcmDataLength) {
                    bufferSize = pcmDataLength;
                    pcmBuffer = static_cast<uint8_t *>(realloc(pcmBuffer, bufferSize));
                }
                bool rendered = false;
                if (mRenderingCb) {
                    rendered = mRenderingCb(mRenderingCbUserData, filter_frame.get());
                }
                if (!mMute && !rendered) {
                    copyPCMData(getAVFrame(filter_frame.get()), pcmBuffer);
                } else {
                    memset(pcmBuffer, 0, pcmDataLength);
                }
                SDL_QueueAudio(mDevID, pcmBuffer, pcmDataLength);
                mPlayedDuration += filter_frame->getInfo().duration;
            }

            return mFiler->push(frame, 0);
        }

//    AF_LOGD("nb samples is %d\n", frame->getInfo().audio.nb_samples);
        int pcmDataLength = getPCMDataLen(frame->getInfo().audio.channels, 
                                          (enum AVSampleFormat) frame->getInfo().audio.format,
                                          frame->getInfo().audio.nb_samples);
        if (bufferSize < pcmDataLength) {
            bufferSize = pcmDataLength;
            pcmBuffer = static_cast<uint8_t *>(realloc(pcmBuffer, bufferSize));
        }
        bool rendered = false;
        if (mRenderingCb) {
            rendered = mRenderingCb(mRenderingCbUserData, frame.get());
        }
        if (!mMute && !rendered) {
            copyPCMData(getAVFrame(frame.get()), pcmBuffer);
        } else {
            memset(pcmBuffer, 0, pcmDataLength);
        }
        SDL_QueueAudio(mDevID, pcmBuffer, pcmDataLength);
        assert(frame->getInfo().duration > 0);
        mPlayedDuration += frame->getInfo().duration;
//    AF_LOGD("queued duration is %llu\n", getQueDuration());
        frame = nullptr;
        return 0;
    }

    inline uint64_t SdlAFAudioRender::getQueDuration()
    {
        double byteRate = mSpec.channels * 4 * mSpec.freq;
        return (uint64_t) ((SDL_GetQueuedAudioSize(mDevID) / byteRate) * 1000000);
    }

    void SdlAFAudioRender::SDLAudioCallback(void *userdata, Uint8 *stream, int len)
    {
        auto *audioRender = static_cast<SdlAFAudioRender *>(userdata);
        SDL_memset(stream, 0, len);

        if (len == 0) {
            return;
        }

        if (audioRender->bufferSize == audioRender->mStartPos) {
            if (audioRender->mAudioFrames.empty()) {
                return;
            }

            audioRender->mStartPos = 0;
            copyPCMData(getAVFrame(audioRender->mAudioFrames.front().get()), audioRender->pcmBuffer);
            audioRender->bufferSize = getPCMDataLen(audioRender->mAudioFrames.front()->getInfo().audio.channels,
                                                    (enum AVSampleFormat) audioRender->mAudioFrames.front()->getInfo().audio.format,
                                                    audioRender->mAudioFrames.front()->getInfo().audio.nb_samples);
            audioRender->mAudioFrames.pop();
        }

        int leftSize = audioRender->bufferSize - audioRender->mStartPos;
        int streamLen = (len > leftSize ? leftSize : len);
        SDL_MixAudio(stream, audioRender->pcmBuffer + audioRender->mStartPos * sizeof(uint8_t), streamLen,
                     SDL_MIX_MAXVOLUME);
        audioRender->mStartPos += streamLen;
    }

    int SdlAFAudioRender::renderFrame(std::unique_ptr<IAFPacket> &packet, int timeOut)
    {
        return 0;
    }

    int64_t SdlAFAudioRender::getPosition()
    {
        return mPlayedDuration - getQueDuration();
    }

    void SdlAFAudioRender::mute(bool bMute)
    {
        if (bMute) {
            SDL_ClearQueuedAudio(mDevID);
        }
        mMute = bMute;
    }

    int SdlAFAudioRender::setVolume(float volume)
    {
        if (volume == mVolume) {
            return 0;
        }

        if (mFiler == nullptr) {
            mFiler = std::unique_ptr<ffmpegAudioFilter>(new ffmpegAudioFilter(mInfo, mInfo, true));
            mFiler->init();
        }

        mFiler->setOption("volume", std::to_string(volume), "volume");
        if (volume < 0.001f) {
            SDL_ClearQueuedAudio(mDevID);
        }
        mVolume = volume;
        return 0;
    }

    int SdlAFAudioRender::setSpeed(float speed)
    {
        if (speed == mSpeed) {
            return 0;
        }

        if (mFiler == nullptr) {
            mFiler = std::unique_ptr<ffmpegAudioFilter>(new ffmpegAudioFilter(mInfo, mInfo, true));
            mFiler->init();
        }

        mFiler->setOption("rate", std::to_string(speed), "atempo");
        mSpeed = speed;
        return 0;
    }

    void SdlAFAudioRender::pause(bool bPause)
    {
        SDL_PauseAudio(bPause);
    }

    void SdlAFAudioRender::flush()
    {
        mPlayedDuration = 0;
    }
}
