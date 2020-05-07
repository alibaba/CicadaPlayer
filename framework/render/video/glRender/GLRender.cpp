//
// Created by lifujun on 2019/8/12.
//
#define  LOG_TAG "GLRender"


#include "render_system/EGL/gl_context_factory.h"
#include "GLRender.h"
#include <utils/timer.h>
#include <utils/AFMediaType.h>
#include <cassert>
#include <cstdlib>
#include <render/video/glRender/base/utils.h>

using namespace std;

static const int MAX_IN_SIZE = 3;

#ifdef __ANDROID__

    #include "OESProgramContext.h"

#elif TARGET_OS_IPHONE

    #include "CV420PProgramContext.h"
    #include <base/media/PBAFFrame.h>

#endif

#include "render/video/vsync/VSyncFactory.h"
#include "YUVProgramContext.h"

using namespace Cicada;

GLRender::GLRender(float Hz)
{
    mVSync = VSyncFactory::create(*this, Hz);
    mHz = 0;
    mVSyncPeriod = static_cast<int64_t>(1000000 / Hz);
#if TARGET_OS_IPHONE
    IOSNotificationManager::Instance()->RegisterObserver(this, 0);
    mInBackground = IOSNotificationManager::Instance()->GetActiveStatus() == 0;
    setenv("METAL_DEVICE_WRAPPER_TYPE", "0", 1);
//   setenv("CG_CONTEXT_SHOW_BACKTRACE", "1", 1);
#endif
}

GLRender::~GLRender()
{
#if TARGET_OS_IPHONE
    IOSNotificationManager::Instance()->RemoveObserver(this);
#endif
    AF_LOGE("~GLRender");
    // MUST delete Vsync here,because it has callback
    mVSync = nullptr;
}

int GLRender::init()
{
    AF_LOGD("-----> init .");
    // don't auto start in background
    std::unique_lock<std::mutex> locker(mInitMutex);

    if (!mInBackground) {
        mVSync->start();
    }

    return 0;
}

int GLRender::clearScreen()
{
    AF_LOGD("-----> clearScreen");
    mClearScreenOn = true;
    return 0;
}

int GLRender::renderFrame(std::unique_ptr<IAFFrame> &frame)
{
//    AF_LOGD("-----> renderFrame");
    if (mInitRet != INT32_MIN && mInitRet != 0) {
        return -EINVAL;
    }

    if (frame == nullptr) {
        // do flush
        mVSync->pause();
        {
            std::unique_lock<std::mutex> locker(mFrameMutex);

            while (!mInputQueue.empty()) {
                dropFrame();
            }
        }
        std::unique_lock<std::mutex> locker(mInitMutex);

        if (!mInBackground) {
            mVSync->start();
        }

        return 0;
    }

    std::unique_lock<std::mutex> locker(mFrameMutex);
    mInputQueue.push(move(frame));
    return 0;
}

void GLRender::dropFrame()
{
    int64_t framePts = mInputQueue.front()->getInfo().pts;
    AF_LOGI("drop a frame pts = %lld ", framePts);
    mInputQueue.front()->setDiscard(true);
    mInputQueue.pop();

    if (mRenderResultCallback != nullptr) {
        mRenderResultCallback(framePts, false);
    }
}

int GLRender::setRotate(IVideoRender::Rotate rotate)
{
    AF_LOGD("-----> setRotate");
    mRotate = rotate;
    return 0;
}

void GLRender::setVideoRotate(Rotate rotate)
{
    mVideoRotate = rotate;
}

int GLRender::setFlip(IVideoRender::Flip flip)
{
    AF_LOGD("-----> setFlip");
    mFlip = flip;
    return 0;
}

int GLRender::setScale(IVideoRender::Scale scale)
{
    AF_LOGD("-----> setScale");
    mScale = scale;
    return 0;
}


void GLRender::setBackgroundColor(uint32_t color)
{
    mBackgroundColor = color;
};

int GLRender::onVSync(int64_t tick)
{
    int ret = onVsyncInner(tick);
#ifdef __ANDROID__
    {
        unique_lock<mutex> lock(mRenderCallbackMutex);
        mRenderCallbackCon.notify_one();
    }
#endif
    return ret;
}

int GLRender::onVsyncInner(int64_t tick)
{
    if (mInitRet == INT32_MIN) {
        VSyncOnInit();

        if (mInitRet == INT32_MIN) {
            return 0;
        } else if (mInitRet != 0) {
            AF_LOGE("VSyncOnInit error");
            return -EINVAL;
        }
    }

    if (mHz == 0) {
        mHz = mVSync->getHz();

        if (mHz == 0) {
            mHz = 60;
        }

        mVSyncPeriod = static_cast<int64_t>(1000000 / mHz);
    }

    {
        std::unique_lock<std::mutex> locker(mFrameMutex);

        if (!mInputQueue.empty()) {
            if (mInputQueue.size() >= MAX_IN_SIZE) {
                while (mInputQueue.size() >= MAX_IN_SIZE) {
                    dropFrame();
                }

                mRenderClock.set(mInputQueue.front()->getInfo().pts);
                mRenderClock.start();
            } else {
                assert(mInputQueue.front() != nullptr);

                if (mRenderClock.get() == 0) {
                    mRenderClock.set(mInputQueue.front()->getInfo().pts);
                    mRenderClock.start();
                }

                int64_t late = mInputQueue.front()->getInfo().pts - mRenderClock.get();

                if (llabs(late) > 100000) {
                    mRenderClock.set(mInputQueue.front()->getInfo().pts);
                } else if (late - mVSyncPeriod * mRenderClock.getSpeed() > 0) {
//                    AF_LOGD("mVSyncPeriod is %lld\n", mVSyncPeriod);
//                    AF_LOGD("mRenderClock.get() is %lld\n", mRenderClock.get());
//                    AF_LOGD("mInputQueue.front()->getInfo().pts is %lld\n", mInputQueue.front()->getInfo().pts);
                    calculateFPS(tick);
                    return 0;
                }
            }
        }
    }

    if (renderActually()) {
        mRenderCount++;
    }

    calculateFPS(tick);
    return 0;
}

void GLRender::calculateFPS(int64_t tick)
{
    if ((tick / uint64_t(mHz)) != mRendertimeS) {
        if (mRendertimeS == 0 || 1) {
            mRendertimeS = tick / uint64_t(mHz);
        } else {
            mRendertimeS++;
        }

        AF_LOGD("video fps is %llu\n", mRenderCount);
        mFps = mRenderCount;
        mRenderCount = 0;
    }
}

int GLRender::VSyncOnInit()
{
    if (mInBackground) {
        return 0;
    }

    mContext = GLContextFactory::NewInstance();
    mInitRet = mContext->Init(nullptr);
    mInitCondition.notify_all();

    if (mInitRet != 0) {
        AF_LOGE("GLContext init failed. ret = %d ", mInitRet.load());
        return -EINVAL;
    }

    return 0;
}

void GLRender::VSyncOnDestroy()
{
    mPrograms.clear();
    assert(mContext != nullptr);
    mContext->DestroyView();
    mContext->DestroySurface(mGLSurface);
    mGLSurface = nullptr;
    mContext->Destroy();
    delete mContext;
    mContext = nullptr;
}

bool GLRender::renderActually()
{
    if (mInBackground) {
//        AF_LOGD("renderActurally  .. InBackground ..");
        return false;
    }

    //  AF_LOGD("renderActually .");
    bool rendered  = true;
    int64_t renderStartTime = af_getsteady_ms();
#ifdef __ANDROID__

    if (needCreateOutTexture) {
        IProgramContext *programContext = getProgram(AF_PIX_FMT_CICADA_MEDIA_CODEC);
        programContext->createSurface();
        std::unique_lock<std::mutex> locker(mCreateOutTextureMutex);
        needCreateOutTexture = false;
        mCreateOutTextureCondition.notify_all();
    }

#endif
    assert(mContext != nullptr);
    bool displayViewChanged  = false;
    {
        unique_lock<mutex> viewLock(mViewMutex);
        displayViewChanged = mContext->SetView(mDisplayView);
        bool viewSizeChanged = mContext->IsViewSizeChanged();

        if (viewSizeChanged || displayViewChanged
                || (mGLSurface == nullptr && mDisplayView != nullptr)) {
            createGLSurface();
        } else {
            mContext->MakeCurrent(mGLSurface);
        }
    }
    mWindowWidth = mContext->GetWidth();
    mWindowHeight = mContext->GetHeight();

    if (mGLSurface == nullptr) {
//        AF_LOGE("0918 renderActurally  return mGLSurface = null..");
        return false;
    }

    std::unique_ptr<IAFFrame> frame = nullptr;
    {
        std::unique_lock<std::mutex> locker(mFrameMutex);

        if (!mInputQueue.empty()) {
            frame = move(mInputQueue.front());
            mInputQueue.pop();
        } else {
            rendered = false;
        }
    }

    if (frame != nullptr) {
        mProgramFormat = frame->getInfo().format;
        mProgramContext = getProgram(mProgramFormat, frame.get());
    }

    if (mProgramContext == nullptr) {
        mProgramFormat = -1;
        return false;
    }

    int64_t framePts = INT64_MIN;

    if (frame != nullptr) {
        framePts = frame->getInfo().pts;
    }

    Rotate finalRotate = Rotate_None;
    int tmpRotate = (mRotate + mVideoRotate) % 360;

    if (tmpRotate == 0) {
        finalRotate = Rotate_None;
    } else if (tmpRotate == 90) {
        finalRotate = Rotate_90;
    } else if (tmpRotate == 180) {
        finalRotate = Rotate_180;
    } else if (tmpRotate == 270) {
        finalRotate = Rotate_270;
    }

    mProgramContext->updateScale(mScale);
    mProgramContext->updateRotate(finalRotate);
    mProgramContext->updateWindowSize(mWindowWidth, mWindowHeight, displayViewChanged);
    mProgramContext->updateFlip(mFlip);
    mProgramContext->updateBackgroundColor(mBackgroundColor);
    int ret = mProgramContext->updateFrame(frame);
    //work around for glReadPixels is upside-down.
    {
        std::unique_lock<std::mutex> locker(mCaptureMutex);

        if (mCaptureOn && mCaptureFunc != nullptr) {
            //if need capture , update flip and other
            if (mFlip == Flip_None ) {
                mProgramContext->updateFlip(Flip_Vertical);
            } else if (mFlip == Flip_Vertical) {
                mProgramContext->updateFlip(Flip_None);
            } else if ( mFlip == Flip_Horizontal) {
                mProgramContext->updateFlip(Flip_Both);
            }

            if (finalRotate == Rotate_90) {
                mProgramContext->updateRotate(Rotate_270);
            } else if (finalRotate == Rotate_270) {
                mProgramContext->updateRotate(Rotate_90);
            }

            std::unique_ptr<IAFFrame> dummyFrame = nullptr;
            mProgramContext->updateFrame(dummyFrame);
            captureScreen();
            //reset flip and other
            mProgramContext->updateFlip(mFlip);
            mProgramContext->updateRotate(finalRotate);
            mProgramContext->updateFrame(dummyFrame);
        }
    }

    if (ret == 0) {
        //if frame not change, don`t need present surface
        mContext->Present(mGLSurface);

        if ((INT64_MIN != framePts) && (mRenderResultCallback != nullptr)) {
            mRenderResultCallback(framePts, true);
        }
    }

    if (mClearScreenOn) {
        glViewport(0, 0, mWindowWidth, mWindowHeight);
        unsigned int backgroundColor = mBackgroundColor;
        float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        cicada::convertToGLColor(backgroundColor, color);
        glClearColor(color[0], color[1], color[2], color[3]);
        glClear(GL_COLOR_BUFFER_BIT);
        mContext->Present(mGLSurface);

        if (mProgramContext != nullptr) {
            mProgramFormat = -1;
            mProgramContext = nullptr;
        }

        mClearScreenOn = false;
    }

    int64_t end = af_getsteady_ms();

    if (end - renderStartTime > 100) {
        AF_LOGD("renderActually use:%lld", end - renderStartTime);
    }

//   AF_LOGD(" cost time : render = %d ms", (af_getsteady_ms() - renderStartTime));
    return rendered;
}

void GLRender::captureScreen()
{
    int64_t captureStartTime = af_getsteady_ms();
    GLint pView[4];
    glGetIntegerv(GL_VIEWPORT, pView);
    int width = pView[2];
    int height = pView[3];
    GLsizei bufferSize = width * height * sizeof(GLubyte) * 4; //RGBA
    GLubyte *bufferData = (GLubyte *) malloc(bufferSize);
    memset(bufferData, 0, bufferSize);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glReadPixels(pView[0], pView[1], pView[2], pView[3], GL_RGBA, GL_UNSIGNED_BYTE,
                 bufferData);
    int64_t captureEndTime = af_getsteady_ms();
    AF_LOGD("capture cost time : capture = %d ms", (captureEndTime - captureStartTime));
    mCaptureFunc(bufferData, width, height);
    free(bufferData);
    mCaptureOn = false;
}

int GLRender::setDisPlay(void *view)
{
    AF_LOGD("-----> setDisPlay view = %p", view);

    if (mDisplayView != view) {
        mVSync->pause();
        {
            unique_lock<mutex> viewLock(mViewMutex);
            mDisplayView = view;
        }
        std::unique_lock<std::mutex> locker(mInitMutex);

        if (!mInBackground) {
            mVSync->start();
        }
    }

    return 0;
}

void GLRender::createGLSurface()
{
    if (mContext == nullptr) {
        return;
    }

    GLContext *pGLContext = mContext;
    pGLContext->DestroySurface(mGLSurface);
    pGLContext->MakeCurrent(nullptr);
    mGLSurface = pGLContext->CreateSurface();

    if (mGLSurface == nullptr) {
        AF_LOGE("createGLSurface fail ");
    }

    pGLContext->MakeCurrent(mGLSurface);
}

void GLRender::captureScreen(std::function<void(uint8_t *, int, int)> func)
{
    {
        std::unique_lock<std::mutex> locker(mCaptureMutex);
        mCaptureFunc = func;
        mCaptureOn = true;
    }
}

void *GLRender::getSurface()
{
#ifdef __ANDROID__
    {
        std::unique_lock<std::mutex> locker(mCreateOutTextureMutex);
        needCreateOutTexture = true;
        mCreateOutTextureCondition.wait(locker, [this]() -> int {
            return !needCreateOutTexture;
        });
    }
    IProgramContext *programContext = getProgram(AF_PIX_FMT_CICADA_MEDIA_CODEC);

    if (programContext == nullptr) {
        return nullptr;
    }

    return programContext->getSurface();
#endif
    return nullptr;
}

IProgramContext *GLRender::getProgram(int frameFormat, IAFFrame *frame)
{
    if (mPrograms.count(frameFormat) > 0) {
        return mPrograms[frameFormat].get();
    }

    unique_ptr<IProgramContext> targetProgram{nullptr};
#ifdef __ANDROID__

    if (frameFormat == AF_PIX_FMT_CICADA_MEDIA_CODEC) {
        targetProgram = unique_ptr<IProgramContext>(new OESProgramContext());
    } else
#elif TARGET_OS_IPHONE
    if (frameFormat == AF_PIX_FMT_APPLE_PIXEL_BUFFER && frame != nullptr) {
        CVPixelBufferRef pixelBuffer = (dynamic_cast<PBAFFrame *>(frame))->getPixelBuffer();
        OSType pixelFormat = CVPixelBufferGetPixelFormatType(pixelBuffer);

        if (pixelFormat == kCVPixelFormatType_420YpCbCr8BiPlanarFullRange
                || pixelFormat == kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange) {
            targetProgram = unique_ptr<IProgramContext>(new CV420PProgramContext(mContext->GetContext()));
        }
    } else
#endif
        if (frameFormat == AF_PIX_FMT_YUV420P || frameFormat == AF_PIX_FMT_YUVJ420P
                || frameFormat == AF_PIX_FMT_YUV422P || frameFormat == AF_PIX_FMT_YUVJ422P) {
            targetProgram = unique_ptr<IProgramContext>(new YUVProgramContext());
        }

    if (targetProgram == nullptr) {
        return nullptr;
    }

    int ret = targetProgram->initProgram();

    if (ret == 0) {
        mPrograms[frameFormat] = move(targetProgram);
        return mPrograms[frameFormat].get();
    } else {
        return nullptr;
    }
}



void GLRender::setSpeed(float speed)
{
    mRenderClock.setSpeed(speed);
}




#if TARGET_OS_IPHONE

void GLRender::AppWillResignActive()
{
    std::unique_lock<std::mutex> locker(mInitMutex);
    mInBackground = true;
    AF_LOGE("0919, mInBackground = true");
    mVSync->pause();
};

void GLRender::AppDidBecomeActive()
{
    std::unique_lock<std::mutex> locker(mInitMutex);
    mInBackground = false;
    AF_LOGE("0919, mInBackground = false");
    mVSync->start();
}

#endif

float GLRender::getRenderFPS()
{
    return mFps;
}

void GLRender::setRenderResultCallback(function<void(int64_t, bool)> renderResultCallback)
{
    mRenderResultCallback = renderResultCallback;
}

void GLRender::surfaceChanged()
{
#ifdef __ANDROID__

    if (mInitRet == INT32_MIN || mInitRet != 0) {
        return ;
    }

    std::unique_lock<mutex> lock(mRenderCallbackMutex);
    mRenderCallbackCon.wait(lock);
#endif
}

