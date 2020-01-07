//
// Created by moqi on 2019-08-02.
//

#include "AFActiveVideoRender.h"
#include "render/video/vsync/timedVSync.h"

AFActiveVideoRender::AFActiveVideoRender(float Hz)
{
    mVSync = std::unique_ptr<IVSync>(new timedVSync(
    [this](int64_t tick) {
        onVSync(tick);
    }, Hz));
    mVSync->start();
}

int AFActiveVideoRender::setHz(float Hz)
{
    return mVSync->setHz(Hz);
}
