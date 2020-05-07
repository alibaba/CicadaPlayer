//
// Created by lifujun on 2019/8/20.
//
#define LOG_TAG "GLRender_OESContext"

#include <utils/timer.h>
#include <render/video/glRender/base/utils.h>
#include "OESProgramContext.h"

using namespace cicada;

static const char OES_VERTEX_SHADER[] = R"(
        uniform mat4 uMVPMatrix;
        uniform mat4 uSTMatrix;
        attribute vec4 aPosition;
        attribute vec4 aTextureCoord;
        varying vec2 vTextureCoord;
        void main() {
            gl_Position   = uMVPMatrix * aPosition;
            vTextureCoord = (uSTMatrix * aTextureCoord).xy;
        }
)";

static const char OES_FRAGMENT_SHADER[] = R"(
#extension GL_OES_EGL_image_external : require
        precision mediump float;
        varying vec2 vTextureCoord;
        uniform samplerExternalOES sTexture;
        void main() {
            gl_FragColor = texture2D(sTexture, vTextureCoord);
        }
)";


OESProgramContext::OESProgramContext() {
    AF_LOGD("OESProgramContext");
    updateFlipCoords();
    updateDrawRegion();
}

OESProgramContext::~OESProgramContext() {
    AF_LOGD("~OESProgramContext");
    glDisableVertexAttribArray(mPositionLocation);
    glDisableVertexAttribArray(mTexCoordLocation);
    glDetachShader(mOESProgram, mVertShader);
    glDetachShader(mOESProgram, mFragmentShader);
    glDeleteShader(mVertShader);
    glDeleteShader(mFragmentShader);
    glDeleteTextures(1, &mOutTextureId);
    glDeleteProgram(mOESProgram);
    if (mDecoderSurface != nullptr) {
        delete mDecoderSurface;
        mDecoderSurface = nullptr;
    }

}

int OESProgramContext::initProgram() {
    AF_LOGD("createProgram ");
    mOESProgram = glCreateProgram();

    int mInitRet = compileShader(&mVertShader, OES_VERTEX_SHADER, GL_VERTEX_SHADER);

    if (mInitRet != 0) {
        AF_LOGE("compileShader mVertShader failed. ret = %d ", mInitRet);
        return mInitRet;
    }

    mInitRet = compileShader(&mFragmentShader, OES_FRAGMENT_SHADER, GL_FRAGMENT_SHADER);

    if (mInitRet != 0) {
        AF_LOGE("compileShader mFragmentShader failed. ret = %d ", mInitRet);
        return mInitRet;
    }

    glAttachShader(mOESProgram, mVertShader);
    glAttachShader(mOESProgram, mFragmentShader);
    glLinkProgram(mOESProgram);

    GLint status;
    glGetProgramiv(mOESProgram, GL_LINK_STATUS, &status);

    if (status != GL_TRUE) {
        int length = 0;
        GLchar glchar[256] = {0};
        glGetProgramInfoLog(mOESProgram, 256, &length, glchar);
        AF_LOGW("linkProgram  error is %s \n", glchar);
        return -1;
    }

    glUseProgram(mOESProgram);
    getShaderLocations();

    glEnableVertexAttribArray(mPositionLocation);
    glEnableVertexAttribArray(mTexCoordLocation);

    return 0;
}

void OESProgramContext::updateScale(IVideoRender::Scale scale) {
    if (mScale != scale) {
        mScale = scale;
        mRegionChanged = true;
    }
}

void OESProgramContext::updateRotate(IVideoRender::Rotate rotate) {
    if (mRotate != rotate) {
        mRotate = rotate;
        mRegionChanged = true;
    }
}

void OESProgramContext::updateWindowSize(int width, int height, bool windowChanged) {
    mWindowChanged = windowChanged;

    if (mWindowWidth == width && mWindowHeight == height && !mWindowChanged) {
        return;
    }

    mWindowWidth = width;
    mWindowHeight = height;
    mRegionChanged = true;
}

void *OESProgramContext::getSurface() {
    if (mOutTextureId <= 0 || mDecoderSurface == nullptr) {
        return nullptr;
    }

    return mDecoderSurface->GetSurface();
}

void OESProgramContext::updateDrawRegion() {

    if (mWindowWidth == 0 || mWindowHeight == 0 || mFrameWidth == 0 || mFrameHeight == 0) {
        mDrawRegion[0] = (GLfloat) 1.0f;
        mDrawRegion[1] = (GLfloat) -1.0f;
        mDrawRegion[2] = (GLfloat) 0.0f;

        mDrawRegion[3] = (GLfloat) -1.0f;
        mDrawRegion[4] = (GLfloat) -1.0f;
        mDrawRegion[5] = (GLfloat) 0.0f;

        mDrawRegion[6] = (GLfloat) 1.0f;
        mDrawRegion[7] = (GLfloat) 1.0f;
        mDrawRegion[8] = (GLfloat) 0.0f;

        mDrawRegion[9] = (GLfloat) -1.0f;
        mDrawRegion[10] = (GLfloat) 1.0f;
        mDrawRegion[11] = (GLfloat) 0.0f;

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
            off_x = (windowWidth - w);
        } else {
            h = static_cast<int>(scale_w * realHeight);
            off_y = (windowHeight - h);
        }
    } else if (mScale == IVideoRender::Scale::Scale_AspectFill) {
        if (scale_w < scale_h) {
            w = static_cast<int>(scale_h * realWidth);
            off_x = (windowWidth - w);
        } else {
            h = static_cast<int>(scale_w * realHeight);
            off_y = (windowHeight - h);
        }
    }

    float offX = off_x * 1.0f / windowWidth;
    float offY = off_y * 1.0f / windowHeight;

    if (mRotate == IVideoRender::Rotate::Rotate_None) {
        mDrawRegion[0] = (GLfloat) 1.0f - offX;
        mDrawRegion[1] = (GLfloat) -1.0f + offY;
        mDrawRegion[2] = (GLfloat) 0.0f;

        mDrawRegion[3] = (GLfloat) -1.0f + offX;
        mDrawRegion[4] = (GLfloat) -1.0f + offY;
        mDrawRegion[5] = (GLfloat) 0.0f;

        mDrawRegion[6] = (GLfloat) 1.0f - offX;
        mDrawRegion[7] = (GLfloat) 1.0f - offY;
        mDrawRegion[8] = (GLfloat) 0.0f;

        mDrawRegion[9] = (GLfloat) -1.0f + offX;
        mDrawRegion[10] = (GLfloat) 1.0f - offY;
        mDrawRegion[11] = (GLfloat) 0.0f;
    } else if (mRotate == IVideoRender::Rotate::Rotate_90) {

        mDrawRegion[0] = (GLfloat) -1.0f + offX;
        mDrawRegion[1] = (GLfloat) -1.0f + offY;
        mDrawRegion[2] = (GLfloat) 0.0f;

        mDrawRegion[3] = (GLfloat) -1.0f + offX;
        mDrawRegion[4] = (GLfloat) 1.0f - offY;
        mDrawRegion[5] = (GLfloat) 0.0f;

        mDrawRegion[6] = (GLfloat) 1.0f - offX;
        mDrawRegion[7] = (GLfloat) -1.0f + offY;
        mDrawRegion[8] = (GLfloat) 0.0f;

        mDrawRegion[9] = (GLfloat) 1.0f - offX;
        mDrawRegion[10] = (GLfloat) 1.0f - offY;
        mDrawRegion[11] = (GLfloat) 0.0f;

    } else if (mRotate == IVideoRender::Rotate::Rotate_180) {
        mDrawRegion[0] = (GLfloat) -1.0f + offX;
        mDrawRegion[1] = (GLfloat) 1.0f - offY;
        mDrawRegion[2] = (GLfloat) 0.0f;

        mDrawRegion[3] = (GLfloat) 1.0f - offX;
        mDrawRegion[4] = (GLfloat) 1.0f - offY;
        mDrawRegion[5] = (GLfloat) 0.0f;

        mDrawRegion[6] = (GLfloat) -1.0f + offX;
        mDrawRegion[7] = (GLfloat) -1.0f + offY;
        mDrawRegion[8] = (GLfloat) 0.0f;

        mDrawRegion[9] = (GLfloat) 1.0f - offX;
        mDrawRegion[10] = (GLfloat) -1.0f + offY;
        mDrawRegion[11] = (GLfloat) 0.0f;
    } else if (mRotate == IVideoRender::Rotate::Rotate_270) {
        mDrawRegion[0] = (GLfloat) 1.0f - offX;
        mDrawRegion[1] = (GLfloat) 1.0f - offY;
        mDrawRegion[2] = (GLfloat) 0.0f;

        mDrawRegion[3] = (GLfloat) 1.0f - offX;
        mDrawRegion[4] = (GLfloat) -1.0f + offY;
        mDrawRegion[5] = (GLfloat) 0.0f;

        mDrawRegion[6] = (GLfloat) -1.0f + offX;
        mDrawRegion[7] = (GLfloat) 1.0f - offY;
        mDrawRegion[8] = (GLfloat) 0.0f;

        mDrawRegion[9] = (GLfloat) -1.0f + offX;
        mDrawRegion[10] = (GLfloat) -1.0f + offY;
        mDrawRegion[11] = (GLfloat) 0.0f;
    }
}

void OESProgramContext::updateFlipCoords() {
    if (mFlip == IVideoRender::Flip::Flip_Horizontal) {
        mOESFlipCoords[0] = 0.0f;
        mOESFlipCoords[1] = 0.0f;
        mOESFlipCoords[2] = 1.0f;
        mOESFlipCoords[3] = 0.0f;
        mOESFlipCoords[4] = 0.0f;
        mOESFlipCoords[5] = 1.0f;
        mOESFlipCoords[6] = 1.0f;
        mOESFlipCoords[7] = 1.0f;
    } else if (mFlip == IVideoRender::Flip::Flip_Vertical) {
        mOESFlipCoords[0] = 1.0f;
        mOESFlipCoords[1] = 1.0f;
        mOESFlipCoords[2] = 0.0f;
        mOESFlipCoords[3] = 1.0f;
        mOESFlipCoords[4] = 1.0f;
        mOESFlipCoords[5] = 0.0f;
        mOESFlipCoords[6] = 0.0f;
        mOESFlipCoords[7] = 0.0f;
    } else if (mFlip == IVideoRender::Flip::Flip_Both) {
        mOESFlipCoords[0] = 0.0f;
        mOESFlipCoords[1] = 1.0f;
        mOESFlipCoords[2] = 1.0f;
        mOESFlipCoords[3] = 1.0f;
        mOESFlipCoords[4] = 0.0f;
        mOESFlipCoords[5] = 0.0f;
        mOESFlipCoords[6] = 1.0f;
        mOESFlipCoords[7] = 0.0f;
    } else {
        mOESFlipCoords[0] = 1.0f;
        mOESFlipCoords[1] = 0.0f;
        mOESFlipCoords[2] = 0.0f;
        mOESFlipCoords[3] = 0.0f;
        mOESFlipCoords[4] = 1.0f;
        mOESFlipCoords[5] = 1.0f;
        mOESFlipCoords[6] = 0.0f;
        mOESFlipCoords[7] = 1.0f;
    }
}

void OESProgramContext::updateFlip(IVideoRender::Flip flip) {
    if (mFlip != flip) {
        mFlip = flip;
        mCoordsChanged = true;
    }
}

int OESProgramContext::updateFrame(std::unique_ptr<IAFFrame> &frame) {

    if (mOESProgram == 0) {
        return -1;
    }

    if (mDecoderSurface == nullptr) {
        return -1;
    }

    if (frame != nullptr) {
        IAFFrame::videoInfo &videoInfo = frame->getInfo().video;
        if (mFrameWidth != videoInfo.width || mFrameHeight != videoInfo.height ||
            mDar != videoInfo.dar) {
            mDar = videoInfo.dar;
            mFrameWidth = videoInfo.width;
            mFrameHeight = videoInfo.height;
            mRegionChanged = true;
        }
    }

    if (frame == nullptr && !mRegionChanged && !mCoordsChanged) {
        //frame is null and nothing changed , don`t need redraw. such as paused.
//        AF_LOGW("0918, nothing changed");
        return -1;
    }

    frame = nullptr;

    {
        std::unique_lock<std::mutex> waitLock(mFrameAvailableMutex);
        if (!mFrameAvailable) {
            mFrameAvailableCon.wait_for(waitLock, std::chrono::milliseconds(10), [this]() {
                return mFrameAvailable;
            });
        }

        if (mFrameAvailable) {
            mFrameAvailable = false;
        } else if (mWindowChanged) {
            AF_LOGW("frame not available after 10ms");
            return -1;
        }
    }

    if (mRegionChanged) {
        updateDrawRegion();
        mRegionChanged = false;
    }

    if (mCoordsChanged) {
        updateFlipCoords();
        mCoordsChanged = false;
    }

    glVertexAttribPointer(mPositionLocation, 3, GL_FLOAT, GL_FALSE, 12, mDrawRegion);
    glVertexAttribPointer(mTexCoordLocation, 2, GL_FLOAT, GL_FALSE, 8, mOESFlipCoords);

    mDecoderSurface->UpdateTexImg();
    mDecoderSurface->GetTransformMatrix(mOESSTMatrix);

    glUniformMatrix4fv(mMVPMatrixLocation, 1, GL_FALSE, mOESMVMatrix);
    glUniformMatrix4fv(mSTMatrixLocation, 1, GL_FALSE, mOESSTMatrix);
    glUniform1i(mTextureLocation, 0);

    glViewport(0, 0, mWindowWidth, mWindowHeight);

    if(mBackgroundColorChanged) {
        float color[4] = {0.0f,0.0f,0.0f,1.0f};
        cicada::convertToGLColor(mBackgroundColor , color);
        glClearColor(color[0], color[1], color[2], color[3]);
        mBackgroundColorChanged = true;
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mOutTextureId);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    return 0;
}

void OESProgramContext::onFrameAvailable() {
    std::unique_lock<std::mutex> lock(mFrameAvailableMutex);
    mFrameAvailable = true;
}

void OESProgramContext::createSurface() {
    glDeleteTextures(1, &mOutTextureId);
    if (mDecoderSurface != nullptr) {
        delete mDecoderSurface;
    }

    glGenTextures(1, &mOutTextureId);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mOutTextureId);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    mDecoderSurface = new DecoderSurface(this);
    mDecoderSurface->Init(mOutTextureId, nullptr);

    {
        std::unique_lock<std::mutex> lock(mFrameAvailableMutex);
        mFrameAvailable = false;
    }
}

void OESProgramContext::updateBackgroundColor(uint32_t color) {
    if (color != mBackgroundColor) {
        mBackgroundColorChanged = true;
        mBackgroundColor = color;
    }
}

void OESProgramContext::getShaderLocations() {
    mPositionLocation = static_cast<GLuint>(glGetAttribLocation(mOESProgram, "aPosition"));
    mTexCoordLocation = static_cast<GLuint>(glGetAttribLocation(mOESProgram, "aTextureCoord"));
    mMVPMatrixLocation = glGetUniformLocation(mOESProgram, "uMVPMatrix");
    mSTMatrixLocation = glGetUniformLocation(mOESProgram, "uSTMatrix");
    mTextureLocation = glGetUniformLocation(mOESProgram, "sTexture");
}
