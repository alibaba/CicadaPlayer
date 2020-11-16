//
// Created by lifujun on 2019/7/23.
//

#ifndef FRAMEWORK_SDLAFVIDEORENDER_H
#define FRAMEWORK_SDLAFVIDEORENDER_H

#include <SDL2/SDL.h>
#include <mutex>
#include <render/video/AFActiveVideoRender.h>

enum CicadaSDLViewType { CicadaSDLViewType_SDL_WINDOW, CicadaSDLViewType_NATIVE_WINDOW };

typedef struct CicadaSDLView_t {
    void *view;
    CicadaSDLViewType type;
} CicadaSDLView;

class SdlAFVideoRender : public IVideoRender, private IVSync::Listener {
public:
    SdlAFVideoRender();

    ~SdlAFVideoRender() override;

    int init() override;

    int clearScreen() override;

    int renderFrame(std::unique_ptr<IAFFrame> &frame) override;

    int setRotate(Rotate rotate) override;

    int setFlip(Flip flip) override;

    int setScale(Scale scale) override;

    int setDisPlay(void *view) override;

    void captureScreen(std::function<void(uint8_t *data, int width, int height)> func) override;

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

    void onWindowSizeChange(SDL_Window *window);

    int refreshScreen();

    void delayRefreshScreen();

private:
    int VSyncOnInit() override
    {
        return 0;
    };

    void VSyncOnDestroy() override
    {

    }

    int onVSync(int64_t tick) override;

    int onVSyncInner(int64_t tick);

    SDL_Rect getDestRet();

    SDL_Rect getSnapRect();

    SDL_RendererFlip convertFlip();

    void recreateTextureIfNeed(int videoWidth, int videoHeight);


private:
    bool mInited = false;
    SDL_Window *mVideoWindow = nullptr;
    SDL_Texture *mVideoTexture = nullptr;
    SDL_Renderer *mVideoRender = nullptr;
    bool mWindowNeedRelease{false};
    bool mRenderNeedRelease{false};
    void* mCurrentView = nullptr;

    std::unique_ptr<IAFFrame> mLastVideoFrame{};
    std::unique_ptr<IAFFrame> mBackFrame{};

    Rotate mVideoRotate = Rotate::Rotate_None;
    Rotate mRotate = Rotate::Rotate_None;
    Flip mFlip = Flip::Flip_None;
    Scale mScale = Scale::Scale_AspectFit;

    int mVideoWidth = 0;
    int mVideoHeight = 0;

    int mWindowWidth = 720;
    int mWindowHeight = 360;

    std::mutex mRenderMutex;
    std::unique_ptr<IVSync> mVSync{nullptr};
    std::function<void(int64_t, bool)> mRenderResultCallback = nullptr;

#ifdef __WINDOWS__
    std::mutex mWindowSizeChangeMutex{};
    std::condition_variable mWindowSizeChangeCon{};
#endif
};


#endif //FRAMEWORK_SDLAFVIDEORENDER_H
