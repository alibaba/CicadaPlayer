//
// Created by pingkai on 2020/11/27.
//

#ifndef CICADAMEDIA_AVFOUNDATIONVIDEORENDER_H
#define CICADAMEDIA_AVFOUNDATIONVIDEORENDER_H

#include "../AFActiveVideoRender.h"
#include "../IVideoRender.h"
#include "../vsync/IVSync.h"
#include <memory>
#include <utils/pixelBufferConvertor.h>
class DisplayLayerImpl;
class AVFoundationVideoRender : public AFActiveVideoRender {
public:
    AVFoundationVideoRender();
    ~AVFoundationVideoRender() override;
    int init() override;

    int clearScreen() override;

    int setRotate(Rotate rotate) override;

    int setFlip(Flip flip) override;

    int setScale(Scale scale) override;

    int setDisPlay(void *view) override;

private:
    bool deviceRenderFrame(IAFFrame *frame) override;

private:
    std::unique_ptr<DisplayLayerImpl> mRender;
    Cicada::pixelBufferConvertor *mConvertor{nullptr};
};


#endif//CICADAMEDIA_AVFOUNDATIONVIDEORENDER_H
