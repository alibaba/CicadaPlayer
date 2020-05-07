//
// Created by lifujun on 2019/8/12.
//

#ifndef SOURCE_GLRENDER_H
#define SOURCE_GLRENDER_H

#include <render/video/AFActiveVideoRender.h>
#include <mutex>
#include <render/video/vsync/timedVSync.h>
#include <map>
#include <queue>

#include "render_system/EGL/gl_context.h"
#include "platform/platform_gl.h"
#include "IProgramContext.h"

#ifdef __APPLE__

#include <TargetConditionals.h>

#endif

using namespace Cicada;


#if TARGET_OS_IPHONE
#include <codec/utils_ios.h>
class GLRender : public IVideoRender, private IVSync::Listener , private IOSNotificationObserver {
#else
class GLRender : public IVideoRender, private IVSync::Listener {
#endif

public:

    explicit GLRender(float Hz = 60);

    ~GLRender() override;

    int init() override;

    void setVideoRotate(Rotate rotate) override;

    int setDisPlay(void *view) override;

    int clearScreen() override;

    void setBackgroundColor(unsigned int color) override;

    int renderFrame(std::unique_ptr<IAFFrame> &frame) override;

    void setRenderResultCallback(std::function<void(int64_t, bool)> renderResultCallback) override;

    int setRotate(Rotate rotate) override;

    int setFlip(Flip flip) override;

    int setScale(Scale scale) override;

    void setSpeed(float speed) override;

    void captureScreen(std::function<void(uint8_t *, int, int)> func) override;

    void *getSurface() override;

    float getRenderFPS() override;

    void surfaceChanged() override;

private:

    int VSyncOnInit() override;

    int onVSync(int64_t tick) override;

    void VSyncOnDestroy() override;

#if TARGET_OS_IPHONE

    void AppWillResignActive() override;

    void AppDidBecomeActive() override;

#endif

private:

    void dropFrame();

    void createGLSurface();

    bool renderActually();

    void captureScreen();

    void calculateFPS(int64_t tick);

    IProgramContext *getProgram(int frameFormat, IAFFrame *frame = nullptr);

    int onVsyncInner(int64_t tick);

protected:

    std::atomic<Rotate> mVideoRotate{Rotate_None};
    std::atomic<Rotate> mRotate{Rotate_None};
    std::atomic<Flip> mFlip{Flip_None};
    std::atomic<Scale> mScale{Scale_AspectFit};
    std::atomic<uint32_t> mBackgroundColor{0xff000000};

    int mWindowWidth = 0;
    int mWindowHeight = 0;

private:

    std::atomic_int mInitRet{INT32_MIN};
    std::mutex mInitMutex;
    std::condition_variable mInitCondition;
    std::mutex mFrameMutex;
    std::queue<std::unique_ptr<IAFFrame>> mInputQueue;
//    std::unique_ptr<IAFFrame> mLastRenderFrame = nullptr;
    std::mutex mViewMutex;
    void *mDisplayView = nullptr;
    Cicada::GLContext *mContext = nullptr;
    Cicada::GLSurface *mGLSurface = nullptr;
    std::unique_ptr<IVSync> mVSync = nullptr;
    std::mutex mCaptureMutex;
    bool mCaptureOn = false;
    std::function<void(uint8_t *, int, int)> mCaptureFunc = nullptr;
    std::map<int, std::unique_ptr<IProgramContext>> mPrograms;
    std::mutex mCreateOutTextureMutex;
    std::condition_variable mCreateOutTextureCondition;
    bool needCreateOutTexture = false;
    bool mInBackground = false;
    uint64_t mRenderCount{};
    uint64_t mRendertimeS{0};
    uint8_t mFps{0};
    float mHz{};
    int64_t mVSyncPeriod;
    af_scalable_clock mRenderClock;

    IProgramContext *mProgramContext = nullptr;
    int mProgramFormat = -1;

    bool mClearScreenOn = false;

    std::function<void(int64_t,bool)> mRenderResultCallback = nullptr;

#ifdef __ANDROID__
    std::mutex mRenderCallbackMutex{};
    std::condition_variable mRenderCallbackCon{};
#endif

};


#endif //SOURCE_GLRENDER_H
