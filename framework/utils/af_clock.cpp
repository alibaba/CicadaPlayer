//
// Created by moqi on 2018/9/18.
//
#define LOG_TAG "af_clock"

#include "af_clock.h"
#include "timer.h"
#include "frame_work_log.h"

enum timer_status {
    timer_status_init,
    timer_status_start,
    timer_status_pause,
};

af_clock::af_clock()
{
}

af_clock::~af_clock()
{
}

void af_clock::start()
{
    if (mStatus == timer_status_start) {
        return;
    }

    if (mStatus == timer_status_init) {
        mStartUs = af_gettime_relative() - mSetUs;
    } else if (mStatus == timer_status_pause) {
        mStartUs = af_gettime_relative() - mPauseUs;
    }

    mStatus = timer_status_start;
}

void af_clock::pause()
{
    if (mStatus != timer_status_start) {
        return;
    }

    mPauseUs = get();
    mStatus = timer_status_pause;
}

void af_clock::set(int64_t us)
{
    if (mStatus == timer_status_start) {
        mStartUs = af_gettime_relative() - us;
    } else if (mStatus == timer_status_init) {
        mSetUs = us;
    } else if (mStatus == timer_status_pause) {
        mPauseUs = us;
    }
}

int64_t af_clock::get()
{
    switch (mStatus.load()) {
        case timer_status_start:
            return af_gettime_relative() - mStartUs;

        case timer_status_init:
            return 0;

        case timer_status_pause:
            return mPauseUs;

        default:
            return 0;
    }
}

void af_clock::reset()
{
    mStartUs = 0;
    mStatus = 0;
    mSetUs = 0;
    mPauseUs = 0;
}

bool af_clock::isPaused()
{
    return mStatus == timer_status_pause;
}

af_scalable_clock::af_scalable_clock()
{
}

af_scalable_clock::~af_scalable_clock()
{
}

void af_scalable_clock::setSpeed(float speed)
{
    if (speed == mScale) {
        return;
    }

    set(get());
    AF_LOGD("change speed %f --> %f\n", mScale.load(), speed);
    mScale = speed;
}

void af_scalable_clock::reset()
{
    mClock.reset();
    mScale = 1.0f;
    mSetTime = 0;
    mScaleStartTime = 0;
}

void af_scalable_clock::start()
{
    mClock.start();
}

void af_scalable_clock::pause()
{
    mClock.pause();
}

void af_scalable_clock::set(int64_t time)
{
    mSetTime = time;
    mClock.set(0);
}

int64_t af_scalable_clock::get()
{
    return mSetTime + static_cast<int64_t>(mClock.get() * mScale);
}

float af_scalable_clock::getSpeed()
{
    return mScale;
}

bool af_scalable_clock::isPaused()
{
    return mClock.isPaused();
}
