//
// Created by moqi on 2020/1/10.
//

#include <utils/ffmpeg_utils.h>
#include <base/media/AVAFPacket.h>
#include "CheaterAudioRender.h"

#define MAX_INPUT_COUNT 10

using namespace Cicada;

int CheaterAudioRender::init_device()
{
    mOutputInfo.format = AF_SAMPLE_FMT_S16;
    mOutputInfo.sample_rate = 44100;
    mOutputInfo.channels = std::min(mInputInfo.channels, 2);
    mClock.set(0);
    //   mClock.start();
    if (mInputInfo != mOutputInfo) {
        needFilter = true;
    }
    return 0;
}

int CheaterAudioRender::pause_device()
{
    mClock.pause();
    return 0;
}

int CheaterAudioRender::start_device()
{
    mClock.start();
    return 0;
}

void CheaterAudioRender::flush_device()
{
    mClock.set(0);
    std::lock_guard<std::mutex> uMutex(mMutex);
    while (!mQueue.empty()) {
        mQueue.pop();
    }
    mPCMDuration = 0;
}

int64_t CheaterAudioRender::device_get_position()
{
    int64_t clock = mClock.get();
    std::lock_guard<std::mutex> uMutex(mMutex);
    while (!mQueue.empty()) {
        if (clock > mQueue.front()->getPosition()) {
            if (mListener) {
                mListener->onFrameInfoUpdate(mQueue.front()->getFrame()->getInfo(), true);
            }
            if (mRenderingCb) {
                mRenderingCb(mRenderingCbUserData, mQueue.front()->getFrame());
            }
            mQueue.pop();
        } else {
            break;
        }
    }
    if (clock > mPCMDuration) {
        mClock.set(mPCMDuration);
        return mPCMDuration;
    }
    return clock;
}

int CheaterAudioRender::device_write(unique_ptr<IAFFrame> &frame)
{
    if (!frame)
        return 0;

    if (mQueue.size() > MAX_INPUT_COUNT) {
        return -EAGAIN;
    }
    auto *avafFrame = dynamic_cast<AVAFFrame *> (frame.get());
    if (avafFrame) {
        int duration = getPCMFrameDuration(avafFrame->ToAVFrame());
        if (duration > 0)
            mPCMDuration += duration;
    }
    unique_ptr<positionInfo> Info = unique_ptr<positionInfo>(new positionInfo(frame, mPCMDuration));
    std::lock_guard<std::mutex> uMutex(mMutex);
    mQueue.push(std::move(Info));
    return 0;
}

uint64_t CheaterAudioRender::device_get_que_duration()
{
    int64_t clock = mClock.get();
    return std::max((int64_t) 0, mPCMDuration - clock);
}
