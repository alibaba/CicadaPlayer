//
// Created by lifujun on 2019/8/20.
//
#define LOG_TAG "GLRender_OESContext"

#include "OESProgramContext.h"
#include <base/media/AFMediaCodecFrame.h>
#include <base/media/TextureFrame.h>
#include <render/video/glRender/base/utils.h>
#include <utils/CicadaJSON.h>
#include <utils/af_string.h>
#include <utils/timer.h>

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
    glDeleteTextures(1, &mOESTextureId);
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

    getShaderLocations();

    if (mProcessTextureCb != nullptr) {
        textureProcessInitRet = mProcessTextureCb->init(TextureFrame::TEXTURE_RGBA);
    }

    return 0;
}

void OESProgramContext::useProgram()
{
    glUseProgram(mOESProgram);
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
    if (mOESTextureId <= 0 || mDecoderSurface == nullptr) {
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

    float windowWidth = mWindowWidth;
    float windowHeight = mWindowHeight;
    float off_x = 0;
    float off_y = 0;
    float w = mWindowWidth;
    float h = mWindowHeight;
    float realWidth = 0;
    float realHeight = 0;

    if (mRotate == IVideoRender::Rotate::Rotate_90 ||
        mRotate == IVideoRender::Rotate::Rotate_270) {
        realWidth = mFrameHeight;
        realHeight = static_cast<float>(mFrameHeight * mDar);
    } else {
        realWidth = static_cast<float>(mFrameHeight * mDar);
        realHeight = mFrameHeight;
    }

    float scale_w = windowWidth / realWidth;
    float scale_h = windowHeight / realHeight;

    if (mScale == IVideoRender::Scale::Scale_AspectFit) {
        if (scale_w >= scale_h) {
            w = scale_h * realWidth;
            off_x = (windowWidth - w);
        } else {
            h = scale_w * realHeight;
            off_y = (windowHeight - h);
        }
    } else if (mScale == IVideoRender::Scale::Scale_AspectFill) {
        if (scale_w < scale_h) {
            w = scale_h * realWidth;
            off_x = (windowWidth - w);
        } else {
            h = scale_w * realHeight;
            off_y = (windowHeight - h);
        }
    }

    float offX = off_x / windowWidth;
    float offY = off_y / windowHeight;

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

    if (frame == nullptr && !mRegionChanged && !mCoordsChanged && !mBackgroundColorChanged) {
        //frame is null and nothing changed , don`t need redraw. such as paused.
//        AF_LOGW("0918, nothing changed");
        return -1;
    }

    AFMediaCodecFrame *codecFrame = nullptr;
    if (frame != nullptr) {
        codecFrame = dynamic_cast<AFMediaCodecFrame *>(frame.get());
        codecFrame->releaseIndex();
    }

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

    mDecoderSurface->UpdateTexImg();
    mDecoderSurface->GetTransformMatrix(mOESSTMatrix);

    bool rendered = false;
    if (mRenderingCb) {
        CicadaJSONItem params{};
        params.addValue("glContext", (long) mGLContext);
        params.addValue("oesId", (int) mOESTextureId);
        params.addValue("matrix", (long) mOESSTMatrix);
        rendered = mRenderingCb(mRenderingCbUserData, codecFrame, params);
    }

    if (rendered) {
        return -1;
    }

    bool needProcess;
    if (textureProcessInitRet) {
        needProcess = mProcessTextureCb->needProcess();
    } else {
        needProcess = false;
    }

    if (!needProcess) {
        drawTexture(GL_TEXTURE_EXTERNAL_OES, mOESTextureId, false);
        return 0;
    }

    if (mOES2FBOProgram == nullptr) {
        mOES2FBOProgram = new OES2FBOProgram();
        int ret = mOES2FBOProgram->initProgram();
        if (ret < 0) {
            AF_LOGE("OES 2 FBO init fail: %d", ret);
            drawTexture(GL_TEXTURE_EXTERNAL_OES, mOESTextureId, false);
            return 0;
        }
    }

    bool updateFrameBuffer = mOES2FBOProgram->updateFrameBuffer(mFrameWidth, mFrameHeight);
    if (!updateFrameBuffer) {
        drawTexture(GL_TEXTURE_EXTERNAL_OES, mOESTextureId, false);
        return 0;
    }

    GLuint FBOBuffer = mOES2FBOProgram->getFrameBuffer();
    glBindFramebuffer(GL_FRAMEBUFFER, FBOBuffer);
    drawTexture(GL_TEXTURE_EXTERNAL_OES, mOESTextureId, true);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLuint FBOTextureId = mOES2FBOProgram->getFrameTexture();
    GLuint inTexture[1]{FBOTextureId};
    TextureFrame *pFrame = new TextureFrame(TextureFrame::TEXTURE_RGBA, mGLContext, reinterpret_cast<int *>(inTexture), nullptr,
                                            mFrameWidth, mFrameHeight);
    std::unique_ptr<IAFFrame> textureFrame = std::unique_ptr<TextureFrame>(pFrame);
    bool success = mProcessTextureCb->push(textureFrame);

    if (!success) {
        AF_LOGW("process texture fail , will render FBO");
        drawTexture(GL_TEXTURE_2D, FBOTextureId, false);
        return 0;
    }
// TODO pull until no frame
    success = mProcessTextureCb->pull(textureFrame);
    if (!success) {
        AF_LOGW("process texture fail , will render FBO");
        drawTexture(GL_TEXTURE_2D, FBOTextureId, false);

    } else {
        int outTexture = ((TextureFrame *) textureFrame.get())->getTexture()[0];
        drawTexture(GL_TEXTURE_2D, outTexture, false);
    }


    return 0;
}

void OESProgramContext::onFrameAvailable() {
    std::unique_lock<std::mutex> lock(mFrameAvailableMutex);
    mFrameAvailable = true;
}

void OESProgramContext::createSurface() {
    glDeleteTextures(1, &mOESTextureId);
    if (mDecoderSurface != nullptr) {
        delete mDecoderSurface;
    }

    glGenTextures(1, &mOESTextureId);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, mOESTextureId);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    mDecoderSurface = new DecoderSurface(this);
    mDecoderSurface->Init(mOESTextureId, nullptr);

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

void OESProgramContext::drawTexture(GLenum target, GLuint textureId, bool toFBO)
{
    if (target == GL_TEXTURE_EXTERNAL_OES) {
        glUseProgram(mOESProgram);
    } else {
        mOES2FBOProgram->useProgram();
    }

    GLfloat drawRegion[12] = {1.0, -1.0, 0.0, -1.0, -1.0, 0.0, 1.0, 1.0, 0.0, -1.0, 1.0, 0.0};
    GLfloat flipCoords[8] = {1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 1.0};

    int viewPortWith = mFrameWidth;
    int viewPortHeight = mFrameHeight;

    if (target != GL_TEXTURE_EXTERNAL_OES || !toFBO) {

        viewPortWith = mWindowWidth;
        viewPortHeight = mWindowHeight;

        if (mRegionChanged) {
            updateDrawRegion();
            mRegionChanged = false;
        }

        if (mCoordsChanged) {
            updateFlipCoords();
            mCoordsChanged = false;
        }

        memcpy(drawRegion, mDrawRegion, 12 * sizeof(GLfloat));
        memcpy(flipCoords, mOESFlipCoords, 8 * sizeof(GLfloat));
    }

    if (target == GL_TEXTURE_EXTERNAL_OES) {
        glVertexAttribPointer(mPositionLocation, 3, GL_FLOAT, GL_FALSE, 12, drawRegion);
        glEnableVertexAttribArray(mPositionLocation);
        glVertexAttribPointer(mTexCoordLocation, 2, GL_FLOAT, GL_FALSE, 8, flipCoords);
        glEnableVertexAttribArray(mTexCoordLocation);

        glUniformMatrix4fv(mMVPMatrixLocation, 1, GL_FALSE, mOESMVMatrix);
        glUniformMatrix4fv(mSTMatrixLocation, 1, GL_FALSE, mOESSTMatrix);
        glUniform1i(mTextureLocation, 0);
    } else {
        mOES2FBOProgram->enableDrawRegion(drawRegion);
        mOES2FBOProgram->enableFlipCoords(flipCoords);
        mOES2FBOProgram->uniform1i();
    }

    glViewport(0, 0, viewPortWith, viewPortHeight);

    if (mBackgroundColorChanged) {
        float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
        cicada::convertToGLColor(mBackgroundColor, color);
        glClearColor(color[0], color[1], color[2], color[3]);
        mBackgroundColorChanged = false;
    }

    glClear(GL_COLOR_BUFFER_BIT);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(target, textureId);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if (target == GL_TEXTURE_EXTERNAL_OES) {
        glDisableVertexAttribArray(mPositionLocation);
        glDisableVertexAttribArray(mTexCoordLocation);
    } else {
        mOES2FBOProgram->disableDrawRegion();
        mOES2FBOProgram->disableFlipCoords();
    }
    glBindTexture(target, 0);
    glUseProgram(0);
}
