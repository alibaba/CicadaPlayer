//
// Created by lifujun on 2019/9/2.
//

#ifndef CICADA_PLAYER_CVPROGRAMCONTEXT_H
#define CICADA_PLAYER_CVPROGRAMCONTEXT_H


#include "IProgramContext.h"


#include <CoreVideo/CoreVideo.h>

#ifdef __OBJC__
#import <OpenGLES/EAGL.h>
#import <QuartzCore/CAEAGLLayer.h>
#else
struct CAEAGLLayer;
struct EAGLContext;
#endif

class CV420PProgramContext : public IProgramContext {
public:
    explicit CV420PProgramContext(void *context);

    ~CV420PProgramContext() override;

    int initProgram() override;

    void updateScale(IVideoRender::Scale scale) override;

    void updateRotate(IVideoRender::Rotate rotate) override;

    void updateBackgroundColor(uint32_t color) override;

    void updateWindowSize(int width, int height, bool windowChanged) override;

    void updateFlip(IVideoRender::Flip flip) override;
    
    int updateFrame(std::unique_ptr<IAFFrame> &frame) override;

private:

    void getShaderLocations();

    void updateUProjection();

    void updateFlipCoords();

    void updateDrawRegion();

    void updateColorRange();

    void updateColorSpace();
private:
    class Rect{
    public:
        Rect(size_t l,size_t t, size_t r, size_t b){
            left = l;
            top = t;
            right = r;
            bottom = b;
        }
        ~ Rect() = default;
        
    public:
        size_t left;
        size_t top;
        size_t right;
        size_t bottom;
    };
private:
    IVideoRender::Rotate mRotate = IVideoRender::Rotate_None;
    IVideoRender::Flip mFlip = IVideoRender::Flip_None;
    IVideoRender::Scale mScale = IVideoRender::Scale_AspectFit;

    int mWindowWidth = 0;
    int mWindowHeight = 0;

    double mDar = 1;
    int mFrameWidth = 0;
    int mFrameHeight = 0;

    GLuint mCVProgram = 0;
    GLuint mVertShader = 0;
    GLuint mFragmentShader = 0;
    GLint mYLocation = 0;
    GLint mUVLocation = 0;
    GLint mProjectionLocation = 0;
    GLint mColorSpaceLocation = 0;
    GLint mColorRangeLocation = 0;
    GLuint mPositionLocation = 0;
    GLuint mTexCoordLocation = 0;

    bool mProjectionChanged = false;
    GLfloat mUProjection[4][4];
    bool mRegionChanged = false;
    GLfloat mDrawRegion[8] = {0.0f};
    bool mCoordsChanged = false;
    GLfloat mFlipCoords[8] = {0.0f};

    EAGLContext *mContext{nullptr};
    CVOpenGLESTextureCacheRef mTextureCache{nullptr};


    CVOpenGLESTextureRef luminanceTextureRef = nullptr;
    CVOpenGLESTextureRef chrominanceTextureRef = nullptr;

    GLfloat  mUColorSpace[9] = {0.0f};
    int mColorSpace = 0;
    GLfloat  mUColorRange[3] = {0.0f};
    int mColorRange = 0;

    uint32_t mBackgroundColor = 0xff000000;
    bool mBackgroundColorChanged = true;

};


#endif //CICADA_PLAYER_CVPROGRAMCONTEXT_H
