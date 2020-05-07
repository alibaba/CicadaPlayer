//
// Created by lifujun on 2019/8/20.
//

#ifndef SOURCE_YUVPROGRAMCONTEXT_H
#define SOURCE_YUVPROGRAMCONTEXT_H


#include "GLRender.h"

class YUVProgramContext : public IProgramContext {

public:
    YUVProgramContext();
    ~YUVProgramContext() override;

private:
    int initProgram() override ;

    void updateScale(IVideoRender::Scale scale) override ;

    void updateFlip(IVideoRender::Flip flip) override ;

    void updateRotate(IVideoRender::Rotate rotate) override ;

    void updateBackgroundColor(uint32_t color) override;

    void updateWindowSize(int width, int height, bool windowChanged) override ;

    int updateFrame(std::unique_ptr<IAFFrame> &frame) override;

private:

    void createYUVTextures();

    void fillDataToYUVTextures(uint8_t **data, int *pLineSize, int format);

    void bindYUVTextures();

    void getShaderLocations();

    void updateUProjection();

    void updateDrawRegion();

    void updateFlipCoords();

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
    IVideoRender::Scale  mScale  = IVideoRender::Scale_AspectFit;
    IVideoRender::Flip   mFlip   = IVideoRender::Flip_None;

    Rect   mCropRect{0,0,0,0};
    int mYLineSize = 0;

    GLuint  mProgram = 0;
    GLuint mVertShader = 0;
    GLuint mFragmentShader = 0;

    GLint mProjectionLocation;
    GLint mColorSpaceLocation;
    GLint mColorRangeLocation;
    GLuint mPositionLocation;
    GLuint mTexCoordLocation;
    GLuint mYUVTextures[3];
    GLint mYTexLocation;
    GLint mUTexLocation;
    GLint mVTexLocation;

    bool mProjectionChanged = false;
    GLfloat mUProjection[4][4];
    bool mRegionChanged = false;
    GLfloat mDrawRegion[8] = {0.0f};
    bool mCoordsChanged = false;
    GLfloat mFlipCoords[8] = {0.0f};

    int mWindowWidth  = 0;
    int mWindowHeight = 0;

    double mDar = 1;
    int mFrameWidth  = 0;
    int mFrameHeight = 0;

    GLfloat  mUColorSpace[9] = {0.0f};
    int mColorSpace = 0;
    GLfloat  mUColorRange[3] = {0.0f};
    int mColorRange = 0;

    uint32_t mBackgroundColor = 0xff000000;
    bool mBackgroundColorChanged = true;

};


#endif //SOURCE_YUVPROGRAMCONTEXT_H
