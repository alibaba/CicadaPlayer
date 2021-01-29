//
// Created by moqi on 2019-08-02.
//

#define LOG_TAG "AFActiveVideoRender"
#include "AFActiveVideoRender.h"
#include "render/video/vsync/timedVSync.h"
#include <utils/frame_work_log.h>

#define MAX_FRAME_QUEUE_SIZE 3
#define MAX_IN_SIZE 3
using namespace std;

AFActiveVideoRender::AFActiveVideoRender(float Hz)
#if 0
    : mInputQueue(MAX_FRAME_QUEUE_SIZE)
#endif
{
    mVSync = std::unique_ptr<IVSync>(new timedVSync(*this, Hz));
    mVSync->start();
}

AFActiveVideoRender::~AFActiveVideoRender()
{}

int AFActiveVideoRender::setHz(float Hz)
{
    return mVSync->setHz(Hz);
}
void AFActiveVideoRender::setSpeed(float speed)
{
    mRenderClock.setSpeed(speed);
}
int AFActiveVideoRender::renderFrame(std::unique_ptr<IAFFrame> &frame)
{
    if (frame == nullptr) {
        // TODO: flush
        return 0;
    }
    std::unique_lock<std::mutex> locker(mFrameMutex);
    mInputQueue.push(move(frame));
    return 0;
}

void AFActiveVideoRender::dropFrame()
{
    int64_t framePts = mInputQueue.front()->getInfo().pts;
    AF_LOGI("drop a frame pts = %lld ", framePts);
    mInputQueue.front()->setDiscard(true);
    mInputQueue.pop();

    if (mRenderResultCallback != nullptr) {
        mRenderResultCallback(framePts, false);
    }
}

void AFActiveVideoRender::calculateFPS(int64_t tick)
{
    if ((tick / uint64_t(mVSync->getHz())) != mRendertimeS) {
        mRendertimeS = tick / uint64_t(mVSync->getHz());
        AF_LOGD("video fps is %llu\n", mRenderCount);
        mFps = mRenderCount;
        mRenderCount = 0;
    }
}
int AFActiveVideoRender::onVSync(int64_t tick)
{
    if (mInputQueue.size() >= MAX_IN_SIZE) {
        while (mInputQueue.size() >= MAX_IN_SIZE) {
            dropFrame();
        }

        mRenderClock.set(mInputQueue.front()->getInfo().pts);
        mRenderClock.start();
    }

    if (mInputQueue.empty()) {
        calculateFPS(tick);
        return 0;
    }

    if (mRenderClock.get() == 0) {
        mRenderClock.set(mInputQueue.front()->getInfo().pts);
        mRenderClock.start();
    }

    int64_t late = mInputQueue.front()->getInfo().pts - mRenderClock.get();

    if (llabs(late) > 100000) {
        mRenderClock.set(mInputQueue.front()->getInfo().pts);
    } else if (late - mVSync->getPeriod() * mRenderClock.getSpeed() > 0) {
        //      AF_LOGD("mVSyncPeriod is %lld\n", mVSyncPeriod);
        //      AF_LOGD("mRenderClock.get() is %lld\n", mRenderClock.get());
        //      AF_LOGD("mInputQueue.front()->getInfo().pts is %lld\n", mInputQueue.front()->getInfo().pts);
        calculateFPS(tick);
        return 0;
    }

    mFrameInfo = mInputQueue.front()->getInfo();
    if (deviceRenderFrame(mInputQueue.front().get())) {
        mRenderCount++;
    }
    calculateFPS(tick);

    if (mRenderResultCallback != nullptr) {
        mRenderResultCallback(mFrameInfo.pts, true);
    }
    if (mListener) {
        mListener->onFrameInfoUpdate(mFrameInfo);
    }
    mInputQueue.pop();
    return 0;
}
