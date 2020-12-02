//
// Created by pingkai on 2020/11/27.
//

#include "AVFoundationVideoRender.h"
#include "DisplayLayerImpl-interface.h"

using namespace std;

AVFoundationVideoRender::AVFoundationVideoRender()
{
    mRender = static_cast<unique_ptr<DisplayLayerImpl>>(new DisplayLayerImpl());
    mRender->init();
    mRender->createLayer();
}
AVFoundationVideoRender::~AVFoundationVideoRender()
{}
int AVFoundationVideoRender::init()
{
    return 0;
}
int AVFoundationVideoRender::clearScreen()
{
    return 0;
}
int AVFoundationVideoRender::renderFrame(std::unique_ptr<IAFFrame> &frame)
{
    return mRender->renderFrame(frame);
}
void AVFoundationVideoRender::setRenderResultCallback(std::function<void(int64_t, bool)> renderedCallback)
{}
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
    return 0;
}
