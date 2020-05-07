//
// Created by lifujun on 2019/7/23.
//

#define  LOG_TAG "SdlAFVideoRender"

#include <render/video/vsync/VSyncFactory.h>
#include "SdlAFVideoRender.h"


SdlAFVideoRender::SdlAFVideoRender()
{
    mVSync = VSyncFactory::create(*this, 60);
//   mHz = 0;
    mVSync->start();
};

SdlAFVideoRender::~SdlAFVideoRender()
{
    if (mVideoTexture != nullptr) {
        SDL_DestroyTexture(mVideoTexture);
    }

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


int SdlAFVideoRender::init()
{
    int initRet = SDL_Init(SDL_INIT_VIDEO);

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
    SDL_GetWindowSize(mVideoWindow, &mWindowWidth, &mWindowHeight);
    SDL_GetRendererOutputSize(mVideoRender, &renderWidth, &renderHeight);
    float DISPLAY_WIDTH_RATIO = renderWidth * 1.0f / mWindowWidth;
    float DISPLAY_HEIGHT_RATIO = renderHeight * 1.0f / mWindowHeight;
    SDL_RenderSetScale(mVideoRender, DISPLAY_WIDTH_RATIO, DISPLAY_HEIGHT_RATIO);
    SDL_setenv(SDL_HINT_RENDER_SCALE_QUALITY, "linear", 0);
    return 0;
}


int SdlAFVideoRender::refreshScreen()
{
    {
        std::unique_lock<std::mutex> lock(mRenderMutex);

        if (mLastVideoFrame == nullptr && mBackFrame != nullptr) {
            mLastVideoFrame = mBackFrame->clone();
        }
    }
    onVSync(-1);
    return 0;
}


int SdlAFVideoRender::clearScreen()
{
    std::unique_lock<std::mutex> lock(mRenderMutex);

    if (mVideoRender != nullptr && mVideoTexture != nullptr) {
        SDL_SetRenderDrawColor(mVideoRender, 0, 0, 0, 255);
        SDL_RenderClear(mVideoRender);
        SDL_RenderPresent(mVideoRender);
    }

    return 0;
}

int SdlAFVideoRender::renderFrame(std::unique_ptr<IAFFrame> &frame)
{
    {
        std::unique_lock<std::mutex> lock(mRenderMutex);
        if (frame == nullptr) {
            mVSync->pause();
        }
        if (mLastVideoFrame && mRenderResultCallback) {
            mLastVideoFrame->setDiscard(true);
            mRenderResultCallback(mLastVideoFrame->getInfo().pts, false);
        }
        mLastVideoFrame = std::move(frame);
        if (frame == nullptr) {
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
    std::unique_ptr<IAFFrame> frame;
    {
        std::unique_lock<std::mutex> lock(mRenderMutex);

        if (mLastVideoFrame == nullptr) {
            return 0;
        }

        frame = move(mLastVideoFrame);
    }
    IAFFrame::videoInfo &videoInfo = frame->getInfo().video;
    recreateTextureIfNeed(videoInfo.width, videoInfo.height);
    SDL_Rect srcRect{};
    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.w = mVideoWidth;
    srcRect.h = mVideoHeight;
    int angle = mRotate;
    SDL_RendererFlip flip = convertFlip();
    SDL_Rect dstRect = getDestRet();
    {
        std::unique_lock<std::mutex> lock(mRenderMutex);
        uint8_t **data = frame->getData();
        int *lineSize = frame->getLineSize();

        if (mVideoRender != nullptr && mVideoTexture != nullptr) {
            SDL_UpdateYUVTexture(mVideoTexture, &srcRect,
                                 data[0], lineSize[0],
                                 data[1], lineSize[1],
                                 data[2], lineSize[2]);
            SDL_RenderClear(mVideoRender);
            SDL_RenderCopyEx(mVideoRender,//SDL_Renderer*          renderer,
                             mVideoTexture,//SDL_Texture*           texture,
                             &srcRect,//const SDL_Rect*        srcrect,
                             &dstRect,//const SDL_Rect*        dstrect,
                             angle, //const double           angle,
                             nullptr,//const SDL_Point*       center,
                             flip//const SDL_RendererFlip flip
            );
            SDL_RenderPresent(mVideoRender);
        }
    }
    {
        std::unique_lock<std::mutex> lock(mRenderMutex);

        if (mRenderResultCallback)
            mRenderResultCallback(frame->getInfo().pts, true);
        mBackFrame = move(frame);
    }
    return 0;
}


int SdlAFVideoRender::setRotate(Rotate rotate)
{
    mRotate = rotate;
    return 0;
}

void SdlAFVideoRender::setVideoRotate(Rotate rotate)
{
    mVideoRotate = rotate;
}

int SdlAFVideoRender::setFlip(Flip flip)
{
    mFlip = flip;
    return 0;
}

int SdlAFVideoRender::setScale(Scale scale)
{
    mScale = scale;
    return 0;
}

SDL_Rect SdlAFVideoRender::getDestRet()
{
    SDL_Rect dstRect{};
    SDL_GetWindowSize(mVideoWindow, &mWindowWidth, &mWindowHeight);

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

void SdlAFVideoRender::setWindowSize(int windWith, int windHeight)
{
    if (mWindowWidth != windWith || mWindowHeight != windHeight) {
        mWindowWidth = windWith;
        mWindowHeight = windHeight;
        refreshScreen();
    }
}
/*
std::unique_ptr<IVideoRender::ScreenShotInfo> SdlAFVideoRender::screenShot()
{
    {
        std::unique_lock<std::mutex> lock(mRenderMutex);

        if (mVideoRender == nullptr) {
            return nullptr;
        }
    }
    SDL_Rect finalRect = getSnapRect();
    SDL_Surface *surface = SDL_CreateRGBSurface(0, finalRect.w, finalRect.h, 32, 0, 0, 0, 0);

    if (surface == nullptr) {
        AF_LOGE("Texture could not be created! SDL_Error: %s\n", SDL_GetError());
        return nullptr;
    }

    Uint32 surfaceFormat = surface->format->format;
    {
        std::unique_lock<std::mutex> lock(mRenderMutex);
        SDL_RenderReadPixels(mVideoRender, &finalRect, surfaceFormat, surface->pixels, surface->pitch);
    }
    ScreenShotInfo::Format format = ScreenShotInfo::Format::UNKNOWN;
    size_t size = 0;
    char *pixBuf = nullptr;

    if (surfaceFormat == SDL_PIXELFORMAT_RGB888) {
        format = ScreenShotInfo::Format::RGB888;
        size = static_cast<size_t>(3 * finalRect.w * finalRect.h);
        pixBuf = static_cast<char *>(SDL_malloc(size));
        SDL_memcpy(pixBuf, surface->pixels, size);
    }

    auto info = new ScreenShotInfo();
    info->format = format;
    info->buf = pixBuf;
    info->bufLen = size;
    info->width = finalRect.w;
    info->height = finalRect.h;
    SDL_FreeSurface(surface);
    return std::unique_ptr<IVideoRender::ScreenShotInfo>(info);
}
 */

SDL_Rect SdlAFVideoRender::getSnapRect()
{
    int width = 0;
    int height = 0;
    SDL_GetRendererOutputSize(mVideoRender, &width, &height);
    float DISPLAY_WIDTH_RATIO = width * 1.0f / mWindowWidth;
    float DISPLAY_HEIGHT_RATIO = height * 1.0f / mWindowHeight;
    SDL_Rect rect = getDestRet();
    int finalW, finalH;

    if (mRotate == Rotate::Rotate_None || mRotate == Rotate::Rotate_180) {
        finalW = rect.w;
        finalH = rect.h;
    } else if (mRotate == Rotate::Rotate_90 || mRotate == Rotate::Rotate_270) {
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
    mVideoWindow = static_cast<SDL_Window *>(view);

    if (mVideoWindow) {
        mVideoRender = SDL_GetRenderer(mVideoWindow);
    }

    return 0;
}
