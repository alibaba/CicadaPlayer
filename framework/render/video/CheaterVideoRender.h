//
// Created by moqi on 2020/1/10.
//

#ifndef CICADAMEDIA_CHEATERVIDEORENDER_H
#define CICADAMEDIA_CHEATERVIDEORENDER_H

#include <mutex>
#include "vsync/IVSync.h"
#include "IVideoRender.h"

namespace Cicada {

    class CheaterVideoRender : public IVideoRender, private IVSync::Listener {
    public:
        CheaterVideoRender();

        ~CheaterVideoRender() override;

        int init() override;

        void setWindowSize(int windWith, int mWindHeight) override;

        int clearScreen() override;

        int renderFrame(std::unique_ptr<IAFFrame> &frame) override;

        int setRotate(Rotate rotate) override;

        int setFlip(Flip flip) override;

        int setScale(Scale scale) override;

        int setDisPlay(void *view) override;

/*
    std::unique_ptr<ScreenShotInfo> screenShot() override;
    */

        void setSpeed(float speed) override
        {

        }

        float getRenderFPS() override
        {
            return 0;
        };

        void setRenderResultCallback(std::function<void(int64_t, bool)> renderedCallback) override
        {
            mRenderResultCallback = renderedCallback;
        }

    private:
        int VSyncOnInit() override
        {
            return 0;
        };

        void VSyncOnDestroy() override
        {

        }

        int onVSync(int64_t tick) override;

    private:
        std::function<void(int64_t, bool)> mRenderResultCallback = nullptr;
        std::unique_ptr<IVSync> mVSync{nullptr};
        std::unique_ptr<IAFFrame> mLastVideoFrame{nullptr};
        std::mutex mRenderMutex;
    };
}

#endif //CICADAMEDIA_CHEATERVIDEORENDER_H
