//
// Created by lifujun on 2019/8/20.
//

#ifndef SOURCE_OESRENDER_H
#define SOURCE_OESRENDER_H


#include "platform/android/decoder_surface.h"
#include "GLRender.h"

class OESProgramContext : public IProgramContext , private DecoderSurfaceCallback{
public:
    OESProgramContext();

    ~OESProgramContext() override;

private:
    int initProgram() override;

    void *getSurface() override;

    void updateScale(IVideoRender::Scale scale) override;

    void updateRotate(IVideoRender::Rotate rotate) override;

    void updateWindowSize(int width, int height, bool windowChanged) override;

    void updateFlip(IVideoRender::Flip flip) override ;

    int updateFrame(std::unique_ptr<IAFFrame> &frame) override;

private:

    void createDecoderSurface();

    void updateFlipCoords();

    void updateDrawRegion();

    void onFrameAvailable() override ;
private:

    IVideoRender::Rotate mRotate = IVideoRender::Rotate_None;
    IVideoRender::Flip   mFlip   = IVideoRender::Flip_None;
    IVideoRender::Scale  mScale  = IVideoRender::Scale_AspectFit;

    int mWindowWidth  = 0;
    int mWindowHeight = 0;

    double mDar = 1;
    int mFrameWidth = 0;
    int mFrameHeight = 0;

    GLuint                mOutTextureId    = 0;
    Cicada::DecoderSurface *mDecoderSurface = nullptr;

    GLuint mOESProgram = 0;

    GLfloat mOESMVMatrix[16] = {1.0f, 0, 0, 0,
                                0, 1.0f, 0, 0,
                                0, 0, 1.0f, 0,
                                0, 0, 0, 1.0f};

    float mOESSTMatrix[16] = {1.0f, 0, 0, 0,
                              0, 1.0f, 0, 0,
                              0, 0, 1.0f, 0,
                              0, 0, 0, 1.0f};


    bool mCoordsChanged = false;
    GLfloat mOESFlipCoords[8] = {0.0f};

    bool mRegionChanged = false;
    GLfloat mDrawRegion[12]={0.0f};

    std::mutex mFrameAvailableMutex;
    std::condition_variable mFrameAvailableCon;
    bool mFrameAvailable = false;
};


#endif //SOURCE_OESRENDER_H
