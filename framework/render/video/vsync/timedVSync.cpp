#include <utility>

//
// Created by moqi on 2019-08-05.
//
#define LOG_TAG "timedVSync"

#include <utils/timer.h>
#include <utils/frame_work_log.h>
#include "timedVSync.h"
#include "utils/afThread.h"

timedVSync::timedVSync(Listener &listener, float Hz) : IVSync(listener), mPeriod(1000000 / Hz), mHz(Hz)
{
    mThread = std::unique_ptr<afThread>(NEW_AF_THREAD(timedThread));
    mThread->setBeginCallback([this]() {
        return mListener.VSyncOnInit();
    });
    mThread->setEndCallback([this]() {
        mListener.VSyncOnDestroy();
    });
    mClock.start();
    AF_LOGD("timedVSync %p", this);
}

timedVSync::~timedVSync()
{
    AF_LOGD("~timedVSync %p", this);
    mThread = nullptr;
}

void timedVSync::start()
{
    mBPaused = false;
    mThread->start();
}

void timedVSync::pause()
{
    int64_t startTime = af_getsteady_ms();
    mBPaused = true;
    mThread->pause();
    AF_LOGD("timedThread pause use:%d", af_getsteady_ms() - startTime);
}

int timedVSync::timedThread()
{
    if (mBPaused) {
        return 0;
    }

    int ret = mListener.onVSync(mClock.get() / mPeriod);

    if (ret >= 0 && !mBPaused) {
        int delay = mPeriod - (mClock.get()) % mPeriod;
        af_usleep(delay);
    }

    return ret;
}

int timedVSync::setHz(float Hz)
{
    mPeriod = 1000000 / Hz;
    mHz = Hz;
    return 0;
}

