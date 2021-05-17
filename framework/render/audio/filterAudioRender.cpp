//
// Created by lifujun on 2019/8/29.
//
#define LOG_TAG "AudioRender"

#include <utils/frame_work_log.h>

#include "filterAudioRender.h"
#include <base/media/AVAFPacket.h>
#include <cassert>
#include <cerrno>
#include <filter/filterFactory.h>
#include <utils/af_string.h>
#include <utils/ffmpeg_utils.h>
#include <utils/globalSettings.h>
#include <utils/timer.h>

namespace Cicada {

    const static int MAX_INPUT_BUFFER_COUNT = 10;
    const static int MIN_INPUT_BUFFER_COUNT = 2;

    filterAudioRender::filterAudioRender()
    {}

    filterAudioRender::~filterAudioRender()
    {
        unique_lock<mutex> lock(mFrameQueMutex);
        mRunning = false;
        mFrameQueCondition.notify_one();

        if (mRenderThread) {
            mRenderThread->stop();
        }

        while (!mFrameQue.empty()) {
            mFrameQue.pop();
        }
    }

    int filterAudioRender::init(const IAFFrame::audioInfo *info)
    {
        if (mInputInfo.sample_rate != 0) {
            return -EINVAL;
        }

        mOutputInfo = mInputInfo = *info;
        /*
         * the nb_samples may not correct, update the value in renderFrame
         */
        mOutputInfo.nb_samples = 0;
        int ret = init_device();

        uint64_t device_ability = device_get_ability();

        if (!(device_ability & A_FILTER_FLAG_TEMPO) ||
            globalSettings::getSetting()->getProperty("protected.audio.render.hw.tempo") == "OFF") {
            mFilterFlags |= A_FILTER_FLAG_TEMPO;
        }

        if (!(device_ability & A_FILTER_FLAG_VOLUME) || 1) {// device not support volume >1
            mFilterFlags |= A_FILTER_FLAG_VOLUME;
        }

        if (ret < 0) {
            AF_LOGE("subInit failed , ret = %d ", ret);
            mSuccessInitDevice = false;
            return ret;
        }
        mSuccessInitDevice = true;
        if (mOutputInfo.nb_samples > 0) {
            float rate = (float) mInputInfo.sample_rate / mOutputInfo.sample_rate;
            float nb_samples = mOutputInfo.nb_samples /= rate;
            mOutputInfo.nb_samples = nb_samples;
        }

        if (needFilter) {
            mFilter = std::unique_ptr<IAudioFilter>(filterFactory::createAudioFilter(mInputInfo, mOutputInfo, mUseActiveFilter));
            ret = mFilter->init(mFilterFlags);

            if (ret < 0) {
                return ret;
            }
        }
        mRenderThread = std::unique_ptr<afThread>(NEW_AF_THREAD(renderLoop));
        return 0;
    }

    int filterAudioRender::renderFrame(std::unique_ptr<IAFFrame> &frame, int timeOut)
    {
        unique_lock<mutex> lock(mFrameQueMutex);

        //AF_LOGD("mFrameQue.size() is %d\n", mFrameQue.size());

        if (!mSuccessInitDevice) {
            return OPEN_AUDIO_DEVICE_FAILED;
        }

        if (mFrameQue.size() >= mMaxQueSize) {
            return -EAGAIN;
        }

        if (frame->getInfo().audio != mInputInfo) {
            return FORMAT_NOT_SUPPORT;
        }

        if (mOutputInfo.nb_samples == 0) {
            float rate = (float) mInputInfo.sample_rate / (float) mOutputInfo.sample_rate;
            mOutputInfo.nb_samples = frame->getInfo().audio.nb_samples / rate;
        }

        mFrameQue.push(move(frame));
        mFrameQueCondition.notify_one();
        return 0;
    }

    int filterAudioRender::renderFrame(std::unique_ptr<IAFPacket> &packet, int timeOut)
    {
        return -ENOSPC;
    }

    uint64_t filterAudioRender::getQueDuration()
    {
        uint64_t duration = device_get_que_duration();
        unique_lock<mutex> lock(mFrameQueMutex);

        if (!mFrameQue.empty()) {
            duration += static_cast<uint64_t>(mFrameQue.front()->getInfo().duration * mFrameQue.size());
        }

        return duration;
    }

    int64_t filterAudioRender::getPosition()
    {
        return device_get_position() + static_cast<int64_t>(mSpeedDeltaDuration / (mOutputInfo.sample_rate / 1000000.0f));
    }

    void filterAudioRender::mute(bool bMute)
    {
        mMute = bMute;
        device_mute(bMute);
    }

    int filterAudioRender::setVolume(float volume)
    {
        mVolume = volume;
        return 0;
    }

    int filterAudioRender::setSpeed(float speed)
    {
        if (mFilterFlags & A_FILTER_FLAG_TEMPO) {
            mSpeed = speed;
            return 0;
        }
        return device_setSpeed(speed);
    }

    void filterAudioRender::pause(bool bPause)
    {
        if (bPause) {
            pauseThread();
            pause_device();
        } else {
            startThread();
            start_device();
        }
    }

    int filterAudioRender::pauseThread()
    {
        mRunning = false;
        if (mRenderThread) {
            mRenderThread->pause();
        }
        return 0;
    }


    int filterAudioRender::startThread()
    {
        mRunning = true;
        if (mRenderThread) {
            mRenderThread->start();
        }
        return 0;
    }


    void filterAudioRender::flush()
    {
        bool running = mRunning;
        pauseThread();

        while (!mFrameQue.empty()) {
            mFrameQue.pop();
        }

        if (mFilter) {
            mFilter->flush();
        }

        mMaxQueSize = 2;

        flush_device();
        mSpeedDeltaDuration = 0;
        mRenderFrame = nullptr;

        //nb_samples could changed when reuse
        mOutputInfo.nb_samples = 0;

        if (running) {
            startThread();
        }
    }

    int filterAudioRender::renderLoop()
    {
        if (!mRunning) {
            return 0;
        }

        if (mRenderFrame == nullptr) {
            mRenderFrame = getFrame();
        }
        int ret = 0;

        while (mRenderFrame != nullptr) {
            if (!mRunning) {
                return 0;
            }

            loopChecker();
            int nb_samples = mRenderFrame->getInfo().audio.nb_samples;
            ret = device_write(mRenderFrame);

            if (ret == -EAGAIN) {

                if (mFrameQue.size() == mMaxQueSize) {
                    mMaxQueSize = std::max(MIN_INPUT_BUFFER_COUNT, mMaxQueSize.load() - 1);
                }
                break;
            }
            if (mOutputInfo.nb_samples > 0) {
                mSpeedDeltaDuration += mOutputInfo.nb_samples - nb_samples;
            }
            mRenderFrame = getFrame();
        }
        if (mFrameQue.empty() || ret == -EAGAIN) {

            // TODO: only on Android xiaomi miui 9?
            mMaxQueSize = std::min(mMaxQueSize.load() + 1, MAX_INPUT_BUFFER_COUNT);
            af_msleep(5);
        }
        return 0;
    }

    unique_ptr<IAFFrame> filterAudioRender::getFrame()
    {
        float speed = mSpeed.load();
        if (speed != mFilterSpeed) {
            applySpeed();
            mFilterSpeed = speed;
        }

        float volume = mVolume.load();
        if (volume != mFilterVolume) {
            applyVolume();
            mFilterVolume = volume;
        }

        unique_ptr<IAFFrame> filter_frame{};

        if (mFilter != nullptr) {
            int ret;
            ret = mFilter->pull(filter_frame, 0);

            if (ret != -EAGAIN) {
                //   return filter_frame;
            }

            unique_lock<mutex> lock(mFrameQueMutex);
            while (!mFrameQue.empty()) {

                mFilter->push(mFrameQue.front(), 0);

                if (mFrameQue.front() == nullptr) {
                    mFrameQue.pop();
                } else
                    break;
            }
        } else {
            unique_lock<mutex> lock(mFrameQueMutex);

            if (!mFrameQue.empty()) {
                filter_frame = move(mFrameQue.front());
                mFrameQue.pop();
            }
        }

        return filter_frame;
    }

    int filterAudioRender::applySpeed()
    {
        if (mFilter == nullptr) {
            mFilter = std::unique_ptr<IAudioFilter>(filterFactory::createAudioFilter(mInputInfo, mOutputInfo, mUseActiveFilter));
            mFilter->setOption("rate", AfString::to_string(mSpeed), "atempo");
            int ret = mFilter->init(mFilterFlags);

            if (ret < 0) {
                return ret;
            }
        } else {
            mFilter->setOption("rate", AfString::to_string(mSpeed), "atempo");
        }

        return 0;
    }

    int filterAudioRender::applyVolume()
    {
        float gain = mVolume * mVolume * mVolume;

        if (gain > 1) {
            //use filter to enlarge, set device volume to 1
            if (mFilter == nullptr) {
                mFilter = std::unique_ptr<IAudioFilter>(filterFactory::createAudioFilter(mInputInfo, mOutputInfo, mUseActiveFilter));
                mFilter->setOption("volume", AfString::to_string(gain), "volume");
                int ret = mFilter->init(mFilterFlags);

                if (ret < 0) {
                    return ret;
                }
            } else {
                mFilter->setOption("volume", AfString::to_string(gain), "volume");
            }
            if (device_get_ability() & A_FILTER_FLAG_VOLUME) {
                device_setVolume(1.0);
            }
        } else {
            if (device_get_ability() & A_FILTER_FLAG_VOLUME) {
                if (mFilter) {
                    mFilter->setOption("volume", AfString::to_string(1.0), "volume");
                }
                device_setVolume(gain);
            } else {
                assert(mFilter);
                mFilter->setOption("volume", AfString::to_string(gain), "volume");
            }
        }
        return 0;
    }
    void filterAudioRender::prePause()
    {
        if (mRenderThread) {
            mRenderThread->prePause();
        }
        device_preClose();
    }
}// namespace Cicada
