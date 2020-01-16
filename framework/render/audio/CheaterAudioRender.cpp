//
// Created by moqi on 2020/1/10.
//

#include <utils/ffmpeg_utils.h>
#include <base/media/AVAFPacket.h>
#include "CheaterAudioRender.h"

int Cicada::CheaterAudioRender::init_device()
{
    mOutputInfo.format = AF_SAMPLE_FMT_S16;
    mOutputInfo.sample_rate = 44100;
    mOutputInfo.channels = std::min(mInputInfo.channels, 2);
    mClock.set(0);
    mClock.start();
    return 0;
}

int Cicada::CheaterAudioRender::pause_device()
{
    mClock.pause();
    return 0;
}

int Cicada::CheaterAudioRender::start_device()
{
    mClock.start();
    return 0;
}

void Cicada::CheaterAudioRender::flush_device()
{
    mClock.set(0);
    mPCMDuration = 0;
}

int64_t Cicada::CheaterAudioRender::device_get_position()
{
    int64_t clock = mClock.get();
    if (clock > mPCMDuration) {
        mClock.set(mPCMDuration);
        return mPCMDuration;
    }
    return clock;
}

int Cicada::CheaterAudioRender::device_write(unique_ptr<IAFFrame> &frame)
{
    if (!frame)
        return 0;
    if (mPCMDuration - mClock.get() > 1000000) {
        return -EAGAIN;
    }
    auto *avafFrame = dynamic_cast<AVAFFrame *> (frame.get());
    if (avafFrame) {
        int duration = getPCMFrameDuration(avafFrame->ToAVFrame());
        if (duration > 0)
            mPCMDuration += duration;
    }
    frame = nullptr;
    return 0;
}

uint64_t Cicada::CheaterAudioRender::device_get_que_duration()
{
    int64_t clock = mClock.get();
    return std::max((int64_t) 0, mPCMDuration - clock);

}
