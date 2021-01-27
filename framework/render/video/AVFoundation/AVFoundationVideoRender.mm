//
// Created by pingkai on 2020/11/27.
//
#define LOG_TAG "AVFoundationVideoRender"
#include "AVFoundationVideoRender.h"
#include "DisplayLayerImpl-interface.h"
#include <utils/frame_work_log.h>
#include <utils/timer.h>

using namespace std;
using namespace Cicada;

AVFoundationVideoRender::AVFoundationVideoRender()
{
    mRender = static_cast<unique_ptr<DisplayLayerImpl>>(new DisplayLayerImpl());
    mRender->init();
    mRender->createLayer();
}
AVFoundationVideoRender::~AVFoundationVideoRender()
{
    delete mConvertor;
}
int AVFoundationVideoRender::init()
{
    mStatisticsFrameTime = af_getsteady_ms();
    return 0;
}
int AVFoundationVideoRender::clearScreen()
{
    mRender->clearScreen();
    return 0;
}
int AVFoundationVideoRender::renderFrame(std::unique_ptr<IAFFrame> &frame)
{
    int64_t pts = INT64_MIN;
    bool rendered = false;
    if (frame) {
        pts = frame->getInfo().pts;
        mFrameInfo = frame->getInfo();
        rendered = true;
        if (mConvertor == nullptr) {
            mConvertor = new pixelBufferConvertor();
        }
        if (dynamic_cast<PBAFFrame *>(frame.get()) == nullptr) {
            IAFFrame *pbafFrame = mConvertor->convert(frame.get());
            frame = unique_ptr<IAFFrame>(pbafFrame);
        }
    }
    mRender->renderFrame(frame);
    if (rendered) {
        mStatisticsFrameCount++;
        int64_t time = af_getsteady_ms() - mStatisticsFrameTime;
        if (time > 1000) {
            float timeS = (float) time / 1000;
            mFPS = mStatisticsFrameCount / timeS;
            mStatisticsFrameCount = 0;
            mStatisticsFrameTime = af_getsteady_ms();
            AF_LOGD("video fps is %f\n", mFPS);
        }
        if (mRenderResultCallback) {
            mRenderResultCallback(pts, true);
        }

        if (mListener) {
            mListener->onFrameInfoUpdate(mFrameInfo);
        }
    }
    return 0;
}
int AVFoundationVideoRender::setRotate(IVideoRender::Rotate rotate)
{
    return 0;
}
int AVFoundationVideoRender::setFlip(IVideoRender::Flip flip)
{
    return 0;
}
int AVFoundationVideoRender::setScale(IVideoRender::Scale scale)
{
    mRender->setScale(scale);
    return 0;
}
void AVFoundationVideoRender::setSpeed(float speed)
{}
int AVFoundationVideoRender::setDisPlay(void *view)
{
    mRender->setDisplay(view);
    return 0;
}
float AVFoundationVideoRender::getRenderFPS()
{
    return mFPS;
}
