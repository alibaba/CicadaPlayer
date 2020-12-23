//
// Created by pingkai on 2020/11/27.
//

#ifndef CICADAMEDIA_AVFOUNDATIONVIDEORENDER_H
#define CICADAMEDIA_AVFOUNDATIONVIDEORENDER_H

#include "../IVideoRender.h"
#include "../vsync/IVSync.h"
#include <memory>
class DisplayLayerImpl;
class AVFoundationVideoRender : public IVideoRender {
public:
    AVFoundationVideoRender();
    ~AVFoundationVideoRender() override;
    int init() override;

    int clearScreen() override;

    int renderFrame(std::unique_ptr<IAFFrame> &frame) override;

    int setRotate(Rotate rotate) override;

    int setFlip(Flip flip) override;

    int setScale(Scale scale) override;

    void setSpeed(float speed) override;

    int setDisPlay(void *view) override;

    float getRenderFPS() override;

private:
    std::unique_ptr<DisplayLayerImpl> mRender;
    uint32_t mStatisticsFrameCount{0};
    int64_t mStatisticsFrameTime{0};
    float mFPS{0};
    IAFFrame::AFFrameInfo mFrameInfo;
};


#endif//CICADAMEDIA_AVFOUNDATIONVIDEORENDER_H
