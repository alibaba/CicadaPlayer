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
        mRender->renderFrame(frame);
        rendered = true;
    }
    if (converted) {
        delete frame;
    }

    return rendered;
}
