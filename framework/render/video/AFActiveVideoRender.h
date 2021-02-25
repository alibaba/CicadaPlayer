//
// Created by moqi on 2019-08-02.
//

#ifndef FRAMEWORK_AFACTIVEVIDEORENDER_H
#define FRAMEWORK_AFACTIVEVIDEORENDER_H


#include "IVideoRender.h"
#include "render/video/vsync/IVSync.h"
#include <atomic>
#include <base/media/spsc_queue.h>
#include <mutex>
#include <queue>
#include <utils/af_clock.h>

class AFActiveVideoRender : public IVideoRender, private IVSync::Listener {

public:
    explicit AFActiveVideoRender(float Hz = 60);

    ~AFActiveVideoRender() override;

    void setSpeed(float speed) final;

    int renderFrame(std::unique_ptr<IAFFrame> &frame) final;

    float getRenderFPS() final
    {
        return mFps;
    }

    void captureScreen(std::function<void(uint8_t *, int, int)> func) override;

private:
    int VSyncOnInit() override
    {
        return 0;
    }

    int onVSync(int64_t tick) override;

    void VSyncOnDestroy() override
    {}

    virtual int setHz(float Hz);

    virtual void device_captureScreen(std::function<void(uint8_t *, int, int)> func)
    {}

private:
    virtual bool deviceRenderFrame(IAFFrame *frame) = 0;


private:
    void dropFrame();
    void calculateFPS(int64_t tick);


private:
    std::unique_ptr<IVSync> mVSync{};

    Cicada::SpscQueue<IAFFrame *> mInputQueue;
    //    std::mutex mFrameMutex;
    //    std::queue<std::unique_ptr<IAFFrame>> mInputQueue;
    af_scalable_clock mRenderClock;
    IAFFrame::AFFrameInfo mFrameInfo{};
    uint64_t mRenderCount{};
    uint64_t mRendertimeS{0};
    uint8_t mFps{0};
    size_t mNeedFlushSize{0};
    std::unique_ptr<IAFFrame> mRendingFrame{nullptr};
    std::atomic_bool mNeedCaptureScreen{false};
    std::function<void(uint8_t *, int, int)> mCAPFunc{nullptr};
};


#endif//FRAMEWORK_AFACTIVEVIDEORENDER_H
