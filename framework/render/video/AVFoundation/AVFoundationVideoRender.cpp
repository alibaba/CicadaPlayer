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
#if TARGET_OS_IPHONE
    RegisterIOSNotificationObserver(this, (int) (IOSResignActive | IOSBecomeActive));
#endif
    mRender = static_cast<unique_ptr<DisplayLayerImpl>>(new DisplayLayerImpl());
    mRender->init();
    mRender->createLayer();
}
AVFoundationVideoRender::~AVFoundationVideoRender()
{
#if TARGET_OS_IPHONE
    RemoveIOSNotificationObserver(this);
#endif
    mVSync->pause();
    delete mConvertor;
}
int AVFoundationVideoRender::init()
{
    return 0;
}
int AVFoundationVideoRender::clearScreen()
{
    mRender->clearScreen();
    return 0;
}
int AVFoundationVideoRender::setRotate(IVideoRender::Rotate rotate)
{
    mRender->setRotate(rotate);
    return 0;
}
int AVFoundationVideoRender::setFlip(IVideoRender::Flip flip)
{
    mRender->setFlip(flip);
    return 0;
}
int AVFoundationVideoRender::setScale(IVideoRender::Scale scale)
{
    mRender->setScale(scale);
    return 0;
}

int AVFoundationVideoRender::setDisPlay(void *view)
{
    mRender->setDisplay(view);
    return 0;
}

void AVFoundationVideoRender::setBackgroundColor(uint32_t color)
{
    mRender->setBackgroundColor(color);
}
bool AVFoundationVideoRender::deviceRenderFrame(IAFFrame *frame)
{
    bool rendered = false;
    bool converted = false;
    if (frame) {
        if (mConvertor == nullptr) {
            mConvertor = new pixelBufferConvertor();
        }
        if (dynamic_cast<PBAFFrame *>(frame) == nullptr) {
            IAFFrame *pbafFrame = mConvertor->convert(frame);
            frame = pbafFrame;
            converted = true;
        }
    }
    if (frame) {

        if (mRenderingCb) {
            CicadaJSONItem params{};
            rendered = mRenderingCb(mRenderingCbUserData, frame, params);
        }

        if (rendered) {
            return false;
        }

        mRender->renderFrame(frame);
        rendered = true;
    }
    if (converted) {
        delete frame;
    }

    return rendered;
}
void AVFoundationVideoRender::device_captureScreen(std::function<void(uint8_t *, int, int)> func)
{
    mRender->captureScreen(func);
}
void AVFoundationVideoRender::AppDidBecomeActive()
{
    oNRedraw();
}
void AVFoundationVideoRender::deviceReDraw()
{
    mRender->reDraw();
}
