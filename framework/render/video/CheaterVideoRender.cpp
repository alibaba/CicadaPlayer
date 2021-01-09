//
// Created by moqi on 2020/1/10.
//

#include <render/video/vsync/VSyncFactory.h>
#include "CheaterVideoRender.h"

Cicada::CheaterVideoRender::CheaterVideoRender()
{
    mVSync = VSyncFactory::create(*this, 60);
    mVSync->start();
};

Cicada::CheaterVideoRender::~CheaterVideoRender() = default;

int Cicada::CheaterVideoRender::init()
{
    return 0;
}

void Cicada::CheaterVideoRender::setWindowSize(int windWith, int mWindHeight)
{
    IVideoRender::setWindowSize(windWith, mWindHeight);
}

int Cicada::CheaterVideoRender::clearScreen()
{
    return 0;
}

int Cicada::CheaterVideoRender::renderFrame(std::unique_ptr<IAFFrame> &frame)
{

    std::unique_lock<std::mutex> lock(mRenderMutex);
    if (mLastVideoFrame && mRenderResultCallback) {
        mRenderResultCallback(mLastVideoFrame->getInfo().pts, true);
    }
    mLastVideoFrame = std::move(frame);
    return 0;
}

int Cicada::CheaterVideoRender::setRotate(IVideoRender::Rotate rotate)
{
    return 0;
}

int Cicada::CheaterVideoRender::setFlip(IVideoRender::Flip flip)
{
    return 0;
}

int Cicada::CheaterVideoRender::setScale(IVideoRender::Scale scale)
{
    return 0;
}

int Cicada::CheaterVideoRender::setDisPlay(void *view)
{
    return 0;
}

int Cicada::CheaterVideoRender::onVSync(int64_t tick)
{
    std::unique_lock<std::mutex> lock(mRenderMutex);
    if (!mLastVideoFrame)
        return 0;
    if (mRenderResultCallback) {
        mRenderResultCallback(mLastVideoFrame->getInfo().pts, true);
    }
    mLastVideoFrame = nullptr;
    return 0;
}
