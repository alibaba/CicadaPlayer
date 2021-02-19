//
// Created by pingkai on 2020/11/27.
//

#ifndef CICADAMEDIA_DISPLAYLAYERIMPL_H
#define CICADAMEDIA_DISPLAYLAYERIMPL_H

#include "../IVideoRender.h"
#import <base/media/IAFPacket.h>
class DisplayLayerImpl {
public:
    DisplayLayerImpl();
    ~DisplayLayerImpl();
    void init();
    int createLayer();
    void setDisplay(void *display);
    int renderFrame(IAFFrame *frame);
    void clearScreen();
    void setScale(IVideoRender::Scale scale);
    void setRotate(IVideoRender::Rotate rotate);
    void setFlip(IVideoRender::Flip flip);
    void setBackgroundColor(uint32_t color);

private:
    void applyRotate();

private:
    void *renderHandle{nullptr};
    int mFrameRotate{0};
    int mRotate{0};
    IVideoRender::Flip mFlip{IVideoRender::Flip_None};

    int mFrameDisplayWidth{0};
    int mFrameDisplayHeight{0};

    int mFrameWidth{0};
    int mFrameHeight{0};
    double mFrameDar{0};
};

#endif//CICADAMEDIA_DISPLAYLAYERIMPL_H
