//
// Created by lifujun on 2019/8/29.
//
#define LOG_TAG "AudioRender"

#include <cerrno>
#include <utils/af_string.h>
#include <utils/ffmpeg_utils.h>
#include <filter/filterFactory.h>
#include <base/media/AVAFPacket.h>
#include "filterAudioRender.h"
#include <utils/timer.h>

namespace Cicada {

    const static int MAX_INPUT_BUFFER_COUNT = 2;

    filterAudioRender::filterAudioRender()
    {
    }

    filterAudioRender::~filterAudioRender()
    {
        unique_lock<mutex> lock(mFrameQueMutex);
        mState = State::state_uninit;
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

        if (ret < 0) {
            AF_LOGE("subInit failed , ret = %d ", ret);
            return ret;
        }

        if (needFilter) {
            mFilter = std::unique_ptr<IAudioFilter>(
                          filterFactory::createAudioFilter(mInputInfo, mOutputInfo));
            ret = mFilter->init();

            if (ret < 0) {
                return ret;
            }
        }

        mState = State::state_init;
        mRenderThread = std::unique_ptr<afThread>(NEW_AF_THREAD(renderLoop));
        return 0;
    }

    int filterAudioRender::renderFrame(std::unique_ptr<IAFFrame> &frame, int timeOut)
    {
        unique_lock<mutex> lock(mFrameQueMutex);

        if (mFrameQue.size() > MAX_INPUT_BUFFER_COUNT) {
            return -EAGAIN;
        }

        if (frame->getInfo().audio != mInputInfo) {
            return FORMAT_NOT_SUPPORT;
        }

        if (mOutputInfo.nb_samples == 0) {
            mOutputInfo.nb_samples = frame->getInfo().audio.nb_samples;
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
        if (bMute) {
            device_setVolume(0);
        } else {
            device_setVolume(mVolume);
        }
    }

    int filterAudioRender::setVolume(float volume)
    {
        if (mVolume != volume) {
            mVolume = volume;
            mVolumeChanged = true;
        }

        return 0;
    }
    int filterAudioRender::setSpeed(float speed)
    {
        if (mSpeed != speed) {
            mSpeed = speed;
            mSpeedChanged = true;
        }

        return 0;
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
        if (mState.load() != state_running) {
            AF_LOGE("Pause occur error state %d", mState.load());
            return -1;
        }

        {
            unique_lock<mutex> lock(mFrameQueMutex);
            mState = state_pause;
            mFrameQueCondition.notify_all();
        }

        mRenderThread->pause();
        return 0;
    }



    int filterAudioRender::startThread()
    {
        if (mState != State::state_init && mState != State::state_pause) {
            AF_LOGE("Start occur error state %d", mState.load());
            return -1;
        }

        mState = state_running;
        mRenderThread->start();
        return 0;
    }


    void filterAudioRender::flush()
    {
        State currentState = mState;

        if (currentState == state_running) {
            pauseThread();
        }

        while (!mFrameQue.empty()) {
            mFrameQue.pop();
        }

        if (mFilter) {
            mFilter->flush();
        }

        flush_device();
        mSpeedDeltaDuration = 0;
        mRenderFrame = nullptr;

        if (currentState == state_running) {
            startThread();
        }
    }

    int filterAudioRender::renderLoop()
    {
        if (mState != State::state_running) {
            return 0;
        }

        if (mSpeedChanged) {
            applySpeed();
            mSpeedChanged = false;
        }

        if (mVolumeChanged) {
            applyVolume();
            mVolumeChanged = false;
        }

        loopChecker();

        if (mRenderFrame == nullptr) {
            mRenderFrame = getFrame();
        }

        if (mRenderFrame == nullptr) {
            af_msleep(5);
            return 0;
        }

        int nb_samples = mRenderFrame->getInfo().audio.nb_samples;
        int ret = device_write(mRenderFrame);

        if (ret == -EAGAIN) {
            af_msleep(5);
            return 0;
        }

        if (mOutputInfo.nb_samples > 0) {
            mSpeedDeltaDuration += mOutputInfo.nb_samples - nb_samples;
        }

        return 0;
    }

    unique_ptr<IAFFrame> filterAudioRender::getFrame()
    {
        unique_ptr<IAFFrame> filter_frame{};

        if (mFilter != nullptr) {
            int ret;
            ret = mFilter->pull(filter_frame, 0);

            if (ret != -EAGAIN) {
                return filter_frame;
            }

            unique_lock<mutex> lock(mFrameQueMutex);

            if (mFrameQue.empty()) {
                return filter_frame;
            }

            mFilter->push(mFrameQue.front(), 0);

            if (mFrameQue.front() == nullptr) {
                mFrameQue.pop();
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
            mFilter = std::unique_ptr<IAudioFilter>(
                          filterFactory::createAudioFilter(mInputInfo, mOutputInfo));
            mFilter->setOption("rate", AfString::to_string(mSpeed), "atempo");
            int ret = mFilter->init();

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
        float volume = mVolume;

        if (volume > 1) {
            float gain = volume * volume * volume;

            if (mFilter == nullptr) {
                mFilter = std::unique_ptr<IAudioFilter>(
                              filterFactory::createAudioFilter(mInputInfo, mOutputInfo));
                mFilter->setOption("volume", AfString::to_string(gain), "volume");
                int ret = mFilter->init();

                if (ret < 0) {
                    return ret;
                }
            } else {
                mFilter->setOption("volume", AfString::to_string(gain), "volume");
            }

//set device volume to 1
            volume = 1;
        } else {
            if (mFilter) {
                mFilter->setOption("volume", AfString::to_string(1.0), "volume");
            }
        }

        float gain = volume * volume * volume;
        device_setVolume(gain);
        return 0;
    }
}
