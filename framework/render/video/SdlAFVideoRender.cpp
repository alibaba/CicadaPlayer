//
// Created by lifujun on 2019/7/23.
//

#define  LOG_TAG "SdlAFVideoRender"

#include "SdlAFVideoRender.h"
#include <base/media/AVAFPacket.h>
#include <render/video/vsync/VSyncFactory.h>
#include <thread>
#include <utils/frame_work_log.h>
#ifdef __APPLE__
#include <base/media/PBAFFrame.h>
#endif

static int SDLCALL SdlWindowSizeEventWatch(void *userdata, SDL_Event *event);

static void sdlLogCb(void *userdata, int category, SDL_LogPriority priority, const char *message)
{
    AF_LOGI("sdl log: %d %d %s", category, priority, message);
}

SdlAFVideoRender::SdlAFVideoRender()
{
    mVSync = VSyncFactory::create(*this, 60);
//   mHz = 0;
    SDL_InitSubSystem(SDL_INIT_VIDEO);
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
    SDL_LogSetOutputFunction(sdlLogCb, nullptr);
    mVSync->start();
};

SdlAFVideoRender::~SdlAFVideoRender()
{
    if (mVideoTexture != nullptr) {
        SDL_DestroyTexture(mVideoTexture);
        mVideoTexture = nullptr;
        mInited = false;
    }
    if (mRenderNeedRelease) {
        SDL_DelEventWatch(SdlWindowSizeEventWatch, this);
        SDL_DestroyRenderer(mVideoRender);
        mVideoRender = nullptr;
        mRenderNeedRelease = false;
    }
    if (mWindowNeedRelease && mVideoWindow) {
        SDL_DestroyWindow(mVideoWindow);
        mVideoWindow = nullptr;
        mWindowNeedRelease = false;
    }
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


int SdlAFVideoRender::init()
{
    int initRet = 0;

    if (initRet < 0) {
        AF_LOGE("SdlAFVideoRender could not initialize! Error: %s\n", SDL_GetError());
        return initRet;
    }

    if (mVideoWindow == nullptr) {
        AF_LOGE("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    if (mVideoRender == nullptr) {
        AF_LOGE("Render could not be created! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    int renderWidth = 0;
    int renderHeight = 0;
    SDL_GL_GetDrawableSize(mVideoWindow, &mWindowWidth, &mWindowHeight);
    SDL_GetRendererOutputSize(mVideoRender, &renderWidth, &renderHeight);
    float DISPLAY_WIDTH_RATIO = renderWidth * 1.0f / mWindowWidth;
    float DISPLAY_HEIGHT_RATIO = renderHeight * 1.0f / mWindowHeight;
    SDL_RenderSetScale(mVideoRender, DISPLAY_WIDTH_RATIO, DISPLAY_HEIGHT_RATIO);
    SDL_setenv(SDL_HINT_RENDER_SCALE_QUALITY, "linear", 0);
    return 0;
}


int SdlAFVideoRender::refreshScreen()
{
    bool needClearScreen = false;
    {
        std::unique_lock<std::mutex> lock(mRenderMutex);

        if (mLastVideoFrame == nullptr && mBackFrame != nullptr) {
            mLastVideoFrame = mBackFrame->clone();
        }
        if (mLastVideoFrame == nullptr) {
            needClearScreen = true;
        }
    }
    if (needClearScreen) {
        clearScreen();
    } else {
        onVSync(-1);
    }
    return 0;
}

void SdlAFVideoRender::delayRefreshScreen()
{
    std::thread thread([=]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        {
            std::unique_lock<std::mutex> lock(mRenderMutex);
            if (mLastVideoFrame == nullptr && mBackFrame != nullptr) {
                mLastVideoFrame = mBackFrame->clone();
            }
        }
    });
    thread.detach();
}


int SdlAFVideoRender::clearScreen()
{
    std::unique_lock<std::mutex> lock(mRenderMutex);

    if (mVideoRender != nullptr && mVideoTexture != nullptr) {
        SDL_SetRenderDrawColor(mVideoRender, 0, 0, 0, 255);
        SDL_RenderClear(mVideoRender);
        SDL_RenderPresent(mVideoRender);
    }
    mBackFrame = nullptr;

    return 0;
}

int SdlAFVideoRender::renderFrame(std::unique_ptr<IAFFrame> &frame)
{
    {

        bool paused = false;
        if (frame == nullptr) {
            mVSync->pause();
            paused = true;
        }
        {
            std::unique_lock<std::mutex> lock(mRenderMutex);
            if (mLastVideoFrame && mRenderResultCallback) {
                mLastVideoFrame->setDiscard(true);
                mRenderResultCallback(mLastVideoFrame->getInfo().pts, false);
            }
            if (mListener && mLastVideoFrame) {
                mListener->onFrameInfoUpdate(mLastVideoFrame->getInfo());
            }
            mLastVideoFrame = std::move(frame);
        }
        if (mLastVideoFrame && mVideoRotate != getRotate(mLastVideoFrame->getInfo().video.rotate)) {
            mVideoRotate = getRotate(mLastVideoFrame->getInfo().video.rotate);
        }
        if (paused) {
            mVSync->start();
        }
    }
//    int width = frame->getInfo().video.width;
//    int height = frame->getInfo().video.height;
//    AF_LOGD("video width = %d , height = %d", width, height);
    return 0;
}


void SdlAFVideoRender::recreateTextureIfNeed(int videoWidth, int videoHeight)
{
    if (mInited && videoHeight == mVideoHeight && videoWidth == mVideoWidth) {
//        AF_LOGE("SdlAFVideoRender has initialize!");
        return;
    }

    {
        std::unique_lock<std::mutex> lock(mRenderMutex);
        mInited = true;

        if (mVideoTexture != nullptr) {
            SDL_DestroyTexture(mVideoTexture);
            mVideoTexture = nullptr;
        }

        if (mVideoTexture == nullptr) {
            mVideoTexture = SDL_CreateTexture(mVideoRender, SDL_PIXELFORMAT_IYUV,
                                              SDL_TEXTUREACCESS_STREAMING,
                                              videoWidth, videoHeight);
        }

        if (mVideoTexture == nullptr) {
            AF_LOGE("Texture could not be created! SDL_Error: %s\n", SDL_GetError());
            return;
        }

        mVideoWidth = videoWidth;
        mVideoHeight = videoHeight;
    }
}

int SdlAFVideoRender::onVSync(int64_t tick)
{
    int ret = onVSyncInner(tick);
#ifdef __WINDOWS__
    {
        std::unique_lock<std::mutex> lock(mWindowSizeChangeMutex);
        mWindowSizeChangeCon.notify_one();
    }
#endif
    return ret;
}

int SdlAFVideoRender::onVSyncInner(int64_t tick)
{
    std::unique_ptr<IAFFrame> frame;
    {
        std::unique_lock<std::mutex> lock(mRenderMutex);
        if (mLastVideoFrame == nullptr) {
            return 0;
        }

        frame = move(mLastVideoFrame);
    }
#ifdef __APPLE__
    auto *pBFrame = dynamic_cast<PBAFFrame *>(frame.get());
    if (pBFrame) {
        auto *avafFrame = static_cast<AVAFFrame *>(*pBFrame);
        if (avafFrame) {
            frame = std::unique_ptr<IAFFrame>(avafFrame);
        } else {
            return 0;
        }
    }
#endif
    IAFFrame::videoInfo &videoInfo = frame->getInfo().video;
    recreateTextureIfNeed(videoInfo.width, videoInfo.height);
    SDL_Rect srcRect{};
    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.w = mVideoWidth;
    srcRect.h = mVideoHeight;
    int angle = (mRotate + mVideoRotate) % 360;
    SDL_RendererFlip flip = convertFlip();
    SDL_Rect dstRect = getDestRet();
    {
        std::unique_lock<std::mutex> lock(mRenderMutex);
        uint8_t **data = frame->getData();
        int *lineSize = frame->getLineSize();

        if (mVideoRender != nullptr && mVideoTexture != nullptr) {
            SDL_UpdateYUVTexture(mVideoTexture, &srcRect, data[0], lineSize[0], data[1], lineSize[1], data[2], lineSize[2]);
            SDL_RenderClear(mVideoRender);
            SDL_RenderCopyEx(mVideoRender, //SDL_Renderer*          renderer,
                             mVideoTexture,//SDL_Texture*           texture,
                             &srcRect,     //const SDL_Rect*        srcrect,
                             &dstRect,     //const SDL_Rect*        dstrect,
                             angle,        //const double           angle,
                             nullptr,      //const SDL_Point*       center,
                             flip          //const SDL_RendererFlip flip
            );
            SDL_RenderPresent(mVideoRender);
        }
    }
    {
        std::unique_lock<std::mutex> lock(mRenderMutex);

        if (mRenderResultCallback) mRenderResultCallback(frame->getInfo().pts, true);
        mBackFrame = move(frame);
    }
    return 0;
}


int SdlAFVideoRender::setRotate(Rotate rotate)
{
    mRotate = rotate;
    refreshScreen();
    return 0;
}

int SdlAFVideoRender::setFlip(Flip flip)
{
    mFlip = flip;
    refreshScreen();
    return 0;
}

int SdlAFVideoRender::setScale(Scale scale)
{
    mScale = scale;
    refreshScreen();
    return 0;
}

SDL_Rect SdlAFVideoRender::getDestRet()
{
    SDL_Rect dstRect{};
    SDL_GL_GetDrawableSize(mVideoWindow, &mWindowWidth, &mWindowHeight);
    if (mWindowWidth == 0 || mWindowHeight == 0 ||
            mVideoWidth == 0 || mVideoHeight == 0) {
        dstRect.x = 0;
        dstRect.y = 0;
        dstRect.w = 0;
        dstRect.h = 0;
        return dstRect;
    }

    float videoWhRatio = 1.0f * mVideoWidth / mVideoHeight;
    float windowWhRatio = 1.0f * mWindowWidth / mWindowHeight;
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

    if (videoWhRatio <= windowWhRatio) {
        if (finalRotate == Rotate_None || finalRotate == Rotate_180) {
            if (mScale == Scale::Scale_AspectFit) {
                dstRect.h = mWindowHeight;
                dstRect.w = static_cast<int>(dstRect.h * videoWhRatio);
            } else if (mScale == Scale_AspectFill) {
                dstRect.w = mWindowWidth;
                dstRect.h = static_cast<int>(dstRect.w / videoWhRatio);
            } else if (mScale == Scale::Scale_Fill) {
                dstRect.h = mWindowHeight;
                dstRect.w = mWindowWidth;
            }
        } else if (finalRotate == Rotate_90 || finalRotate == Rotate_270) {
            if (mScale == Scale::Scale_AspectFit) {
                dstRect.w = mWindowHeight;
                dstRect.h = static_cast<int>( dstRect.w / videoWhRatio);
            } else if (mScale == Scale_AspectFill) {
                dstRect.h = mWindowWidth;
                dstRect.w = static_cast<int>( dstRect.h * videoWhRatio);
            } else if (mScale == Scale::Scale_Fill) {
                dstRect.h = mWindowWidth;
                dstRect.w = mWindowHeight;
            }
        }
    } else {
        if (finalRotate == Rotate_None || finalRotate == Rotate_180) {
            if (mScale == Scale::Scale_AspectFit) {
                dstRect.w = mWindowWidth;
                dstRect.h = static_cast<int>(dstRect.w / videoWhRatio);
            } else if (mScale == Scale_AspectFill) {
                dstRect.h = mWindowHeight;
                dstRect.w = static_cast<int>(dstRect.h * videoWhRatio);
            } else if (mScale == Scale::Scale_Fill) {
                dstRect.h = mWindowHeight;
                dstRect.w = mWindowWidth;
            }
        } else if (finalRotate == Rotate_90 || finalRotate == Rotate_270) {
            if (mScale == Scale::Scale_AspectFit) {
                dstRect.h = mWindowWidth;
                dstRect.w = static_cast<int>(dstRect.h * videoWhRatio);
            } else if (mScale == Scale_AspectFill) {
                dstRect.w = mWindowHeight;
                dstRect.h = static_cast<int>(dstRect.w / videoWhRatio);
            } else if (mScale == Scale::Scale_Fill) {
                dstRect.h = mWindowWidth;
                dstRect.w = mWindowHeight;
            }
        }
    }

    dstRect.y = (mWindowHeight - dstRect.h) / 2;
    dstRect.x = (mWindowWidth - dstRect.w) / 2;
    return dstRect;
}

SDL_RendererFlip SdlAFVideoRender::convertFlip()
{
    SDL_RendererFlip flip = SDL_FLIP_NONE;

    if (mFlip == Flip_None) {
        flip = SDL_FLIP_NONE;
    } else if (mFlip == Flip_Horizontal) {
        flip = SDL_FLIP_HORIZONTAL;
    } else if (mFlip == Flip_Vertical) {
        flip = SDL_FLIP_VERTICAL;
    }

    return flip;
}

void SdlAFVideoRender::captureScreen(std::function<void(uint8_t *data, int width, int height)> func)
{
    if (func == nullptr) {
        return;
    }
    {
        std::unique_lock<std::mutex> lock(mRenderMutex);

        if (mVideoRender == nullptr) {
            return;
        }
    }
    SDL_Rect finalRect = getSnapRect();
    SDL_Surface *surface = SDL_CreateRGBSurface(0, finalRect.w, finalRect.h, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

    if (surface == nullptr) {
        AF_LOGE("Texture could not be created! SDL_Error: %s\n", SDL_GetError());
        return;
    }
    refreshScreen();
    Uint32 surfaceFormat = surface->format->format;
    {
        std::unique_lock<std::mutex> lock(mRenderMutex);
        SDL_RenderReadPixels(mVideoRender, &finalRect, surfaceFormat, surface->pixels, surface->pitch);
    }
    if (surfaceFormat == SDL_PIXELFORMAT_ARGB8888) {
        size_t size = 0;
        uint8_t *pixBuf = nullptr;
        size = static_cast<size_t>(4 * finalRect.w * finalRect.h);
        pixBuf = static_cast<uint8_t *>(malloc(size));
        SDL_memcpy(pixBuf, surface->pixels, size);
        func(pixBuf, finalRect.w, finalRect.h);
        free(pixBuf);
        pixBuf = nullptr;
    } else {
        func(nullptr, 0, 0);
    }
    SDL_FreeSurface(surface);

    return ;
}


SDL_Rect SdlAFVideoRender::getSnapRect()
{
    int width = 0;
    int height = 0;
    SDL_GetRendererOutputSize(mVideoRender, &width, &height);
    float DISPLAY_WIDTH_RATIO = width * 1.0f / mWindowWidth;
    float DISPLAY_HEIGHT_RATIO = height * 1.0f / mWindowHeight;
    SDL_Rect rect = getDestRet();
    int finalW, finalH;

    int angle = (mRotate + mVideoRotate) % 360;

    if (angle == Rotate::Rotate_None || angle == Rotate::Rotate_180) {
        finalW = rect.w;
        finalH = rect.h;
    } else if (angle == Rotate::Rotate_90 || angle == Rotate::Rotate_270) {
        finalW = rect.h;
        finalH = rect.w;
    } else {
        finalW = rect.w;
        finalH = rect.h;
    }

    SDL_Rect finalRect{};
    finalRect.w = static_cast<int>(finalW * DISPLAY_WIDTH_RATIO);
    finalRect.h = static_cast<int>(finalH * DISPLAY_HEIGHT_RATIO);
    finalRect.x = static_cast<int>((mWindowWidth - finalW) / 2 * DISPLAY_WIDTH_RATIO);
    finalRect.y = static_cast<int>((mWindowHeight - finalH) / 2 * DISPLAY_HEIGHT_RATIO);
    return finalRect;
}

int SdlAFVideoRender::setDisPlay(void *view)
{
    auto *display = static_cast<CicadaSDLView *>(view);
    if (mCurrentView == display->view) {
        return 0;
    }
    mCurrentView = display->view;
    if (mCurrentView == nullptr) {
        return 0;
    }
    if (mVideoTexture != nullptr) {
        SDL_DestroyTexture(mVideoTexture);
        mVideoTexture = nullptr;
        mInited = false;
    }
    if (mRenderNeedRelease) {
        SDL_DelEventWatch(SdlWindowSizeEventWatch, this);
        SDL_DestroyRenderer(mVideoRender);
        mVideoRender = nullptr;
        mRenderNeedRelease = false;
    }
    if (mWindowNeedRelease && mVideoWindow) {
        SDL_DestroyWindow(mVideoWindow);
        mVideoWindow = nullptr;
        mWindowNeedRelease = false;
    }
    if (display->type == CicadaSDLViewType_NATIVE_WINDOW) {
        mVideoWindow = SDL_CreateWindowFrom(display->view);
        SDL_ShowWindow(mVideoWindow);
        mWindowNeedRelease = true;
    } else {
        mVideoWindow = static_cast<SDL_Window *>(display->view);
        mWindowNeedRelease = false;
    }

    if (mVideoWindow) {
        mVideoRender = SDL_GetRenderer(mVideoWindow);
        if (mVideoRender == nullptr) {
            // log all render name
            int renderCount = SDL_GetNumRenderDrivers();
            for (int i = 0; i < renderCount; i++) {
                SDL_RendererInfo renderDriverInfo;
                SDL_GetRenderDriverInfo(i, &renderDriverInfo);
                std::string renderDriverName;
                if (renderDriverInfo.name) {
                    renderDriverName = renderDriverInfo.name;
                }
                AF_LOGI("sdl render%d: %s\n", i, renderDriverName.c_str());
            }

            // add before renderer created, so this callback will be called before renderer's window size change callback
            SDL_AddEventWatch(SdlWindowSizeEventWatch, this);
            Uint32 renderFlags = 0;
#ifdef __WINDOWS__
            SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d11");
            renderFlags = SDL_RENDERER_SOFTWARE;
#endif
            mVideoRender = SDL_CreateRenderer(mVideoWindow, -1, renderFlags);

            // log the render name
            SDL_RendererInfo renderInfo;
            SDL_GetRendererInfo(mVideoRender, &renderInfo);
            std::string renderName;
            if (renderInfo.name) {
                renderName = renderInfo.name;
            }
            AF_LOGI("create sdl render: %s\n", renderName.c_str());

            mRenderNeedRelease = true;
        } else {
            mRenderNeedRelease = false;
        }
    }

    return 0;
}

int SDLCALL SdlWindowSizeEventWatch(void *userdata, SDL_Event *event)
{
    if (event->type == SDL_WINDOWEVENT) {
        if (event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
            SdlAFVideoRender *pSelf = (SdlAFVideoRender *) userdata;
            if (pSelf) {
                SDL_Window *window = SDL_GetWindowFromID(event->window.windowID);
                pSelf->onWindowSizeChange(window);
            }
        } else if (event->window.event == SDL_WINDOWEVENT_RESIZED) {
            // after SDL_WINDOWEVENT_SIZE_CHANGED event, d3d11 recreate resource, then in this event refresh use new d3d11 resource
            SdlAFVideoRender *pSelf = (SdlAFVideoRender *) userdata;
            if (pSelf) {
                pSelf->delayRefreshScreen();
            }
        }
    }
    return 0;
}

void SdlAFVideoRender::onWindowSizeChange(SDL_Window *window)
{
    if (mVideoWindow == window) {
#ifdef __WINDOWS__
        if (!mInited) {
            return;
        }
        // block the renderer's window size change callback(d3d11 recreate resource) until render complete
        std::unique_lock<std::mutex> lock(mWindowSizeChangeMutex);
        mWindowSizeChangeCon.wait(lock);
#endif
    }
}
