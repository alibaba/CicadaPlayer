//
// Created by lifujun on 2019/9/2.
//
#define LOG_TAG "CV420PProgramContext"

#include "CV420PProgramContext.h"

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#include <base/media/PBAFFrame.h>

#include "platform/platform_gl.h"
#include <utils/timer.h>
#include <render/video/glRender/base/utils.h>

static const char CV_VERTEX_SHADER[] = R"(
        attribute vec2 a_position;
        attribute vec2 a_texCoord;
        uniform mat4 u_projection;
        varying vec2 v_texCoord;

        void main() {
            gl_Position = u_projection * vec4(a_position, 0.0, 1.0);
            v_texCoord  = a_texCoord;
        }
)";

const char *NV12_FRAGMENT_SHADER = R"(
#ifdef GL_ES
    precision mediump float;
#endif
        uniform sampler2D uTxtUV;
        uniform sampler2D uTxtY;

        uniform mat3      uColorSpace;
        uniform vec3      uColorRange;

        varying vec2 v_texCoord;
        void main()
        {
            vec3 yuv;
            yuv.x = (texture2D(uTxtY, v_texCoord).r - uColorRange.x / 255.0) * 255.0 / uColorRange.y;
            yuv.y = (texture2D(uTxtUV, v_texCoord).r - 0.5) * 255.0 / uColorRange.z;
            yuv.z = (texture2D(uTxtUV, v_texCoord).a - 0.5) * 255.0 / uColorRange.z;

            gl_FragColor = vec4(uColorSpace * yuv,1);
        }
)";

CV420PProgramContext::CV420PProgramContext(void *context) {
    mContext = static_cast<EAGLContext *>(context);

    updateFlipCoords();
    updateDrawRegion();
    updateUProjection();
    updateColorRange();
    updateColorSpace();
}

CV420PProgramContext::~CV420PProgramContext() {
    glDisableVertexAttribArray(mPositionLocation);
    glDisableVertexAttribArray(mTexCoordLocation);
    glDetachShader(mCVProgram, mVertShader);
    glDetachShader(mCVProgram, mFragmentShader);
    glDeleteShader(mVertShader);
    glDeleteShader(mFragmentShader);
    glDeleteProgram(mCVProgram);

    if (mTextureCache != nullptr) {
        CFRelease(mTextureCache);
    }

    if (luminanceTextureRef != nullptr) {
        CFRelease(luminanceTextureRef);
    }
    if (chrominanceTextureRef != nullptr) {
        CFRelease(chrominanceTextureRef);
    }

}

int CV420PProgramContext::initProgram() {
    AF_LOGD("createProgram ");
    mCVProgram = glCreateProgram();

    int mInitRet = compileShader(&mVertShader, CV_VERTEX_SHADER, GL_VERTEX_SHADER);

    if (mInitRet != 0) {
        AF_LOGE("compileShader mVertShader failed. ret = %d ", mInitRet);
        return mInitRet;
    }

    mInitRet = compileShader(&mFragmentShader, NV12_FRAGMENT_SHADER, GL_FRAGMENT_SHADER);

    if (mInitRet != 0) {
        AF_LOGE("compileShader mFragmentShader failed. ret = %d ", mInitRet);
        return mInitRet;
    }

    glAttachShader(mCVProgram, mVertShader);
    glAttachShader(mCVProgram, mFragmentShader);
    glLinkProgram(mCVProgram);

    GLint status;
    glGetProgramiv(mCVProgram, GL_LINK_STATUS, &status);

    if (status != GL_TRUE) {
        int length = 0;
        GLchar glchar[256] = {0};
        glGetProgramInfoLog(mCVProgram, 256, &length, glchar);
        AF_LOGW("linkProgram  error is %s \n", glchar);
        return -1;
    }

    glUseProgram(mCVProgram);
    getShaderLocations();

    glEnableVertexAttribArray(mPositionLocation);
    glEnableVertexAttribArray(mTexCoordLocation);

    return 0;
}

void CV420PProgramContext::updateScale(IVideoRender::Scale scale) {
    if (mScale != scale) {
        mScale = scale;
        mRegionChanged = true;
    }
}

void CV420PProgramContext::updateFlip(IVideoRender::Flip flip) {
    if (mFlip != flip) {
        mFlip = flip;
        mCoordsChanged = true;
    }
}

void CV420PProgramContext::updateRotate(IVideoRender::Rotate rotate) {
    if (mRotate != rotate) {
        mRotate = rotate;
        mRegionChanged = true;
    }
}

void CV420PProgramContext::updateBackgroundColor(uint32_t color) {
    if(color != mBackgroundColor) {
        mBackgroundColorChanged = true;
        mBackgroundColor = color;
    }
}

void CV420PProgramContext::updateWindowSize(int width, int height, bool windowChanged) {
    if (mWindowWidth == width && mWindowHeight == height && !windowChanged) {
        return;
    }
    mWindowWidth = width;
    mWindowHeight = height;

    mRegionChanged = true;
    mProjectionChanged = true;
}

int CV420PProgramContext::updateFrame(std::unique_ptr<IAFFrame> &frame) {
    int64_t t1 = af_getsteady_ms();
    CVPixelBufferRef pixelBuffer = nullptr;
    if (frame != nullptr) {
        pixelBuffer = (dynamic_cast<PBAFFrame *>(frame.get()))->getPixelBuffer();
        OSType pixelFormat = CVPixelBufferGetPixelFormatType(pixelBuffer);
        if (pixelFormat != kCVPixelFormatType_420YpCbCr8BiPlanarFullRange
            && pixelFormat != kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange) {
            //not 420p
            return -1;
        }

        IAFFrame::videoInfo &videoInfo = frame->getInfo().video;
        if (mFrameWidth != videoInfo.width || mFrameHeight != videoInfo.height ||
            mDar != videoInfo.dar) {
            mDar = videoInfo.dar;
            mFrameWidth = videoInfo.width;
            mFrameHeight = videoInfo.height;
            mRegionChanged = true;
        }

        if(mColorSpace != videoInfo.colorSpace){
            updateColorSpace();
            mColorSpace = videoInfo.colorSpace;
        }

        if(mColorRange != videoInfo.colorRange){
            updateColorRange();
            mColorRange = videoInfo.colorRange;
        }
    }

    if (frame == nullptr && !mProjectionChanged && !mRegionChanged && !mCoordsChanged) {
        //frame is null and nothing changed , don`t need redraw. such as paused.
        return -1;
    }

    if (mProjectionChanged) {
        updateUProjection();
        mProjectionChanged = false;
    }

    if (mRegionChanged) {
        updateDrawRegion();
        mRegionChanged = false;
    }

    if (mCoordsChanged) {
        updateFlipCoords();
        mCoordsChanged = false;
    }

    int64_t t2 = af_getsteady_ms();
    glViewport(0, 0, mWindowWidth, mWindowHeight);
    if(mBackgroundColorChanged) {
        float color[4]={0.0f,0.0f,0.0f,1.0f};
        cicada::convertToGLColor(mBackgroundColor , color);
        glClearColor(color[0], color[1], color[2], color[3]);
        mBackgroundColorChanged = false;
    }
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(mCVProgram);

    int64_t t3 = af_getsteady_ms();
    if (mTextureCache == nullptr) {
        CVReturn err = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, mContext, NULL, &mTextureCache);
        if (err) {
            AF_LOGE("CVOpenGLESTextureCacheCreate fail %d", err);
            return -1;
        }
    }

    int64_t t4 = af_getsteady_ms();
    if(pixelBuffer != nullptr){
        //new frame , texture should be create new.

        if(luminanceTextureRef != nullptr) {
            CFRelease(luminanceTextureRef);
        }

        if(chrominanceTextureRef != nullptr) {
            CFRelease(chrominanceTextureRef);
        }

        CVReturn ret = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault, mTextureCache, pixelBuffer,
                                                                    NULL,
                                                                    GL_TEXTURE_2D, GL_LUMINANCE, mFrameWidth,
                                                                    mFrameHeight,
                                                                    GL_LUMINANCE, GL_UNSIGNED_BYTE, 0,
                                                                    &luminanceTextureRef);
        if (ret) {
            AF_LOGE("CVOpenGLESTextureCacheCreateTextureFromImage luminanceTextureRef fail %d", ret);
            return -1;
        }

        ret = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault, mTextureCache, pixelBuffer,
                                                                    NULL,
                                                                    GL_TEXTURE_2D, GL_LUMINANCE_ALPHA, mFrameWidth / 2,
                                                                    mFrameHeight / 2, GL_LUMINANCE_ALPHA,
                                                                    GL_UNSIGNED_BYTE, 1,
                                                                    &chrominanceTextureRef);
        if (ret) {
            AF_LOGE("CVOpenGLESTextureCacheCreateTextureFromImage chrominanceTextureRef fail %d", ret);
            return -1;
        }
    }

    {

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(CVOpenGLESTextureGetTarget(luminanceTextureRef), CVOpenGLESTextureGetName(luminanceTextureRef));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glUniform1i(mYLocation, 0);
    }

    {

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(CVOpenGLESTextureGetTarget(chrominanceTextureRef),
                      CVOpenGLESTextureGetName(chrominanceTextureRef));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glUniform1i(mUVLocation, 1);
    }

    int64_t t5 = af_getsteady_ms();
    glUniformMatrix4fv(mProjectionLocation, 1, GL_FALSE, (GLfloat *) mUProjection);
    glUniformMatrix3fv(mColorSpaceLocation, 1, GL_FALSE, (GLfloat *) mUColorSpace);
    glUniform3f(mColorRangeLocation, mUColorRange[0], mUColorRange[1], mUColorRange[2]);
    glVertexAttribPointer(mPositionLocation, 2, GL_FLOAT, GL_FALSE, 0, mDrawRegion);
    glVertexAttribPointer(mTexCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, mFlipCoords);

    int64_t t6 = af_getsteady_ms();
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    int64_t t7 = af_getsteady_ms();
    if (t7 - t1 > 100) {
        AF_LOGD("updateFrame2:%lld 3:%lld 4:%lld 5:%lld 6:%lld 7:%lld", t2-t1, t3-t2, t4-t3, t5-t4,t6-t5,t7-t6);
    }
    return 0;
}


void CV420PProgramContext::updateDrawRegion() {
    if (mWindowWidth == 0 || mWindowHeight == 0 || mFrameWidth == 0 || mFrameHeight == 0) {
        mDrawRegion[0] = (GLfloat) 0;
        mDrawRegion[1] = (GLfloat) 0;
        mDrawRegion[2] = (GLfloat) 0;
        mDrawRegion[3] = (GLfloat) 0;
        mDrawRegion[4] = (GLfloat) 0;
        mDrawRegion[5] = (GLfloat) 0;
        mDrawRegion[6] = (GLfloat) 0;
        mDrawRegion[7] = (GLfloat) 0;
        return;
    }

    int windowWidth = mWindowWidth;
    int windowHeight = mWindowHeight;
    int off_x = 0;
    int off_y = 0;
    int w = mWindowWidth;
    int h = mWindowHeight;
    int realWidth = 0;
    int realHeight = 0;

    if (mRotate == IVideoRender::Rotate::Rotate_90 ||
        mRotate == IVideoRender::Rotate::Rotate_270) {
        realWidth = mFrameHeight;
        realHeight = static_cast<int>(mFrameHeight * mDar);
    } else {
        realWidth = static_cast<int>(mFrameHeight * mDar);
        realHeight = mFrameHeight;
    }

    float scale_w = windowWidth * 1.0f / realWidth;
    float scale_h = windowHeight * 1.0f / realHeight;

    if (mScale == IVideoRender::Scale::Scale_AspectFit) {
        if (scale_w >= scale_h) {
            w = static_cast<int>(scale_h * realWidth);
            off_x = (windowWidth - w) / 2;
        } else {
            h = static_cast<int>(scale_w * realHeight);
            off_y = (windowHeight - h) / 2;
        }
    } else if (mScale == IVideoRender::Scale::Scale_AspectFill) {
        if (scale_w < scale_h) {
            w = static_cast<int>(scale_h * realWidth);
            off_x = (windowWidth - w) / 2;
        } else {
            h = static_cast<int>(scale_w * realHeight);
            off_y = (windowHeight - h) / 2;
        }
    }

    if (mRotate == IVideoRender::Rotate::Rotate_None) {
        mDrawRegion[0] = (GLfloat) (off_x);
        mDrawRegion[1] = (GLfloat) (off_y);
        mDrawRegion[2] = (GLfloat) (off_x + w);
        mDrawRegion[3] = (GLfloat) (off_y);
        mDrawRegion[4] = (GLfloat) (off_x);
        mDrawRegion[5] = (GLfloat) (off_y + h);
        mDrawRegion[6] = (GLfloat) (off_x + w);
        mDrawRegion[7] = (GLfloat) (off_y + h);
    } else if (mRotate == IVideoRender::Rotate::Rotate_90) {
        mDrawRegion[0] = (GLfloat) (off_x);
        mDrawRegion[1] = (GLfloat) (off_y + h);
        mDrawRegion[2] = (GLfloat) (off_x);
        mDrawRegion[3] = (GLfloat) (off_y);
        mDrawRegion[4] = (GLfloat) (off_x + w);
        mDrawRegion[5] = (GLfloat) (off_y + h);
        mDrawRegion[6] = (GLfloat) (off_x + w);
        mDrawRegion[7] = (GLfloat) (off_y);
    } else if (mRotate == IVideoRender::Rotate::Rotate_180) {
        mDrawRegion[0] = (GLfloat) (off_x + w);
        mDrawRegion[1] = (GLfloat) (off_y + h);
        mDrawRegion[2] = (GLfloat) (off_x);
        mDrawRegion[3] = (GLfloat) (off_y + h);
        mDrawRegion[4] = (GLfloat) (off_x + w);
        mDrawRegion[5] = (GLfloat) (off_y);
        mDrawRegion[6] = (GLfloat) (off_x);
        mDrawRegion[7] = (GLfloat) (off_y);
    } else if (mRotate == IVideoRender::Rotate::Rotate_270) {
        mDrawRegion[0] = (GLfloat) (off_x + w);
        mDrawRegion[1] = (GLfloat) (off_y);
        mDrawRegion[2] = (GLfloat) (off_x + w);
        mDrawRegion[3] = (GLfloat) (off_y + h);
        mDrawRegion[4] = (GLfloat) (off_x);
        mDrawRegion[5] = (GLfloat) (off_y);
        mDrawRegion[6] = (GLfloat) (off_x);
        mDrawRegion[7] = (GLfloat) (off_y + h);
    }
}


void CV420PProgramContext::updateFlipCoords() {
    if (mFlip == IVideoRender::Flip::Flip_Horizontal) {
        mFlipCoords[0] = 1.0f;
        mFlipCoords[1] = 1.0f;
        mFlipCoords[2] = 0.0f;
        mFlipCoords[3] = 1.0f;
        mFlipCoords[4] = 1.0f;
        mFlipCoords[5] = 0.0f;
        mFlipCoords[6] = 0.0f;
        mFlipCoords[7] = 0.0f;
    } else if (mFlip == IVideoRender::Flip::Flip_Vertical) {
        mFlipCoords[0] = 0.0f;
        mFlipCoords[1] = 0.0f;
        mFlipCoords[2] = 1.0f;
        mFlipCoords[3] = 0.0f;
        mFlipCoords[4] = 0.0f;
        mFlipCoords[5] = 1.0f;
        mFlipCoords[6] = 1.0f;
        mFlipCoords[7] = 1.0f;
    }else if(mFlip == IVideoRender::Flip::Flip_Both){
        mFlipCoords[0] = 0.0f;
        mFlipCoords[1] = 1.0f;
        mFlipCoords[2] = 1.0f;
        mFlipCoords[3] = 1.0f;
        mFlipCoords[4] = 0.0f;
        mFlipCoords[5] = 0.0f;
        mFlipCoords[6] = 1.0f;
        mFlipCoords[7] = 0.0f;
    } else {
        mFlipCoords[0] = 0.0f;
        mFlipCoords[1] = 1.0f;
        mFlipCoords[2] = 1.0f;
        mFlipCoords[3] = 1.0f;
        mFlipCoords[4] = 0.0f;
        mFlipCoords[5] = 0.0f;
        mFlipCoords[6] = 1.0f;
        mFlipCoords[7] = 0.0f;
    }
}

void CV420PProgramContext::updateUProjection() {
    mUProjection[0][0] = 2.0f;
    mUProjection[0][1] = 0.0f;
    mUProjection[0][2] = 0.0f;
    mUProjection[0][3] = 0.0f;
    mUProjection[1][0] = 0.0f;
    mUProjection[1][1] = 2.0f;
    mUProjection[1][2] = 0.0f;
    mUProjection[1][3] = 0.0f;
    mUProjection[2][0] = 0.0f;
    mUProjection[2][1] = 0.0f;
    mUProjection[2][2] = 0.0f;
    mUProjection[2][3] = 0.0f;
    mUProjection[3][0] = -1.0f;
    mUProjection[3][1] = -1.0f;
    mUProjection[3][2] = 0.0f;
    mUProjection[3][3] = 1.0f;

    if (mWindowHeight != 0 && mWindowWidth != 0) {
        mUProjection[0][0] = 2.0f / mWindowWidth;
        mUProjection[1][1] = 2.0f / mWindowHeight;
    }
}

void CV420PProgramContext::updateColorRange() {
    if (mColorRange == COLOR_RANGE_LIMITIED) {
        mUColorRange[0] = 16;
        mUColorRange[1] = 235 - 16;
        mUColorRange[2] = 240 - 16;
    } else if (mColorRange == COLOR_RANGE_FULL) {
        mUColorRange[0] = 0;
        mUColorRange[1] = 255;
        mUColorRange[2] = 255;
    } else {
        mUColorRange[0] = 16;
        mUColorRange[1] = 235 - 16;
        mUColorRange[2] = 240 - 16;
    }
}

void CV420PProgramContext::updateColorSpace() {
    if (mColorSpace == COLOR_SPACE_BT601) {
        mUColorSpace[0] = 1.f;
        mUColorSpace[1] = 1.f;
        mUColorSpace[2] = 1.f;
        mUColorSpace[3] = 0.0f;
        mUColorSpace[4] = -0.344136f;
        mUColorSpace[5] = 1.772f;
        mUColorSpace[6] = 1.402f;
        mUColorSpace[7] = -0.714136f;
        mUColorSpace[8] = 0.0f;
    } else if (mColorSpace == COLOR_SPACE_BT709) {
        mUColorSpace[0] = 1.f;
        mUColorSpace[1] = 1.f;
        mUColorSpace[2] = 1.f;
        mUColorSpace[3] = 0.0f;
        mUColorSpace[4] = -0.187324f;
        mUColorSpace[5] = 1.8556f;
        mUColorSpace[6] = 1.5748f;
        mUColorSpace[7] = -0.468124f;
        mUColorSpace[8] = 0.0f;
    } else if (mColorSpace == COLOR_SPACE_BT2020) {
        mUColorSpace[0] = 1.f;
        mUColorSpace[1] = 1.f;
        mUColorSpace[2] = 1.f;
        mUColorSpace[3] = 0.0f;
        mUColorSpace[4] = -0.164553f;
        mUColorSpace[5] = 1.8814f;
        mUColorSpace[6] = 1.4746f;
        mUColorSpace[7] = -0.571353f;
        mUColorSpace[8] = 0.0f;
    } else {
        mUColorSpace[0] = 1.f;
        mUColorSpace[1] = 1.f;
        mUColorSpace[2] = 1.f;
        mUColorSpace[3] = 0.0f;
        mUColorSpace[4] = -0.344136f;
        mUColorSpace[5] = 1.772f;
        mUColorSpace[6] = 1.402f;
        mUColorSpace[7] = -0.714136f;
        mUColorSpace[8] = 0.0f;

    }
}

void CV420PProgramContext::getShaderLocations() {
    mYLocation = glGetUniformLocation(mCVProgram, "uTxtY");
    mUVLocation = glGetUniformLocation(mCVProgram, "uTxtUV");
    mProjectionLocation = glGetUniformLocation(mCVProgram, "u_projection");
    mColorSpaceLocation = glGetUniformLocation(mCVProgram, "uColorSpace");
    mColorRangeLocation = glGetUniformLocation(mCVProgram, "uColorRange");
    mPositionLocation = static_cast<GLuint>(glGetAttribLocation(mCVProgram, "a_position"));
    mTexCoordLocation = static_cast<GLuint>(glGetAttribLocation(mCVProgram, "a_texCoord"));
}
