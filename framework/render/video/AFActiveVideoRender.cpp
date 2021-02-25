//
// Created by moqi on 2019-08-02.
//

#define LOG_TAG "AFActiveVideoRender"
#include "AFActiveVideoRender.h"
#include "render/video/vsync/timedVSync.h"
#include <utils/frame_work_log.h>

#define MAX_FRAME_QUEUE_SIZE 100
#define MAX_IN_SIZE 3
using namespace std;

AFActiveVideoRender::AFActiveVideoRender(float Hz)
    : mInputQueue(MAX_FRAME_QUEUE_SIZE)
{
    mVSync = std::unique_ptr<IVSync>(new timedVSync(*this, Hz));
    mVSync->start();
}

AFActiveVideoRender::~AFActiveVideoRender()
{
    if (mRendingFrame) {
        mRendingFrame->setDiscard(true);
    }
    while (mInputQueue.size() > 0) {
        dropFrame();
    }
    mVSync->pause();
    mVSync = nullptr;
}

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
        // flush the mRendingFrame, when mInputQueue is empty
        mNeedFlushSize = std::max(mInputQueue.size(), (size_t) 1);
        return 0;
    }

    if (mNeedFlushSize > 0) {
        // FIXME: can't drop this frame
        AF_LOGW("renderFrame before flush finish\n");
    }

    if (mInputQueue.size() >= MAX_FRAME_QUEUE_SIZE) {
        AF_LOGE("too many frames...\n");
        return 0;
    }
    //    std::unique_lock<std::mutex> locker(mFrameMutex);
    mInputQueue.push(frame.release());
    return 0;
}

void AFActiveVideoRender::dropFrame()
{
    if(mInputQueue.size() <= 0){
        return;
    }
    int64_t framePts = mInputQueue.front()->getInfo().pts;
    AF_LOGI("drop a frame pts = %lld ", framePts);
    mInputQueue.front()->setDiscard(true);
    delete mInputQueue.front();
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
    while (mNeedFlushSize > 0) {
        if (mRendingFrame) {
            mRendingFrame->setDiscard(true);
            mRendingFrame = nullptr;
        }
        if (mInputQueue.empty()) {
            break;
        }
        dropFrame();
        mNeedFlushSize--;
    }
    mNeedFlushSize = 0;

    if (mInputQueue.size() >= MAX_IN_SIZE) {
        while (mInputQueue.size() >= MAX_IN_SIZE) {
            dropFrame();
        }
        mRendingFrame = static_cast<unique_ptr<IAFFrame>>(mInputQueue.front());
        mInputQueue.pop();
        mRenderClock.set(mRendingFrame->getInfo().pts);
        mRenderClock.start();
    }

    if (mInputQueue.empty() && mRendingFrame == nullptr) {
        calculateFPS(tick);
        return 0;
    }

    if (mRendingFrame == nullptr) {
        mRendingFrame = static_cast<unique_ptr<IAFFrame>>(mInputQueue.front());
        mInputQueue.pop();
    }

    if (mRenderClock.get() == 0) {
        mRenderClock.set(mRendingFrame->getInfo().pts);
        mRenderClock.start();
    }

    int64_t late = mRendingFrame->getInfo().pts - mRenderClock.get();

    if (llabs(late) > 100000) {
        mRenderClock.set(mRendingFrame->getInfo().pts);
    } else if (late - mVSync->getPeriod() * mRenderClock.getSpeed() > 0) {
        //        AF_LOGD("mVSyncPeriod is %lld\n", mVSync->getPeriod());
        //        AF_LOGD("mRenderClock.get() is %lld\n", mRenderClock.get());
        //        AF_LOGD("frame->getInfo().pts is %lld\n", mRendingFrame->getInfo().pts);
        calculateFPS(tick);
        return 0;
    }

    mFrameInfo = mRendingFrame->getInfo();
    if (deviceRenderFrame(mRendingFrame.get())) {
        mRenderCount++;
    }
    mRendingFrame = nullptr;
    calculateFPS(tick);

    if (mRenderResultCallback != nullptr) {
        mRenderResultCallback(mFrameInfo.pts, true);
    }
    if (mListener) {
        mListener->onFrameInfoUpdate(mFrameInfo);
    }
    return 0;
}
