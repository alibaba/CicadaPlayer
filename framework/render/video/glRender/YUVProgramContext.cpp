//
// Created by lifujun on 2019/8/20.
//

#define LOG_TAG "GLRender_YUVContext"

#include "YUVProgramContext.h"
#include <utils/frame_work_log.h>
#include <utils/AFMediaType.h>
#include <render/video/glRender/base/utils.h>


static const char YUV_VERTEX_SHADER[] = R"(
        attribute vec2 a_position;
        attribute vec2 a_texCoord;
        uniform mat4 u_projection;
        varying vec2 v_texCoord;

        void main() {
            gl_Position = u_projection * vec4(a_position, 0.0, 1.0);
            v_texCoord  = a_texCoord;
        }
)";

static const char YUV_FRAGMENT_SHADER[] = R"(
#ifdef GL_ES
        precision mediump float;
#endif
        uniform sampler2D y_tex;
        uniform sampler2D u_tex;
        uniform sampler2D v_tex;

        uniform mat3      uColorSpace;
        uniform vec3      uColorRange;

        varying vec2 v_texCoord;

        void main() {
            vec3 yuv;
            vec3 rgb;
            yuv.x = (texture2D(y_tex, v_texCoord).r - uColorRange.x / 255.0) * 255.0 / uColorRange.y;
            yuv.y = (texture2D(u_tex, v_texCoord).r - 0.5) * 255.0 / uColorRange.z;
            yuv.z = (texture2D(v_tex, v_texCoord).r - 0.5) * 255.0 / uColorRange.z;
            rgb = uColorSpace * yuv;
            gl_FragColor = vec4(rgb, 1.0);
        }
)";


YUVProgramContext::YUVProgramContext() {
    AF_LOGD("YUVProgramContext");
    updateDrawRegion();
    updateFlipCoords();
    updateUProjection();
    updateColorRange();
    updateColorSpace();
}

YUVProgramContext::~YUVProgramContext() {
    AF_LOGD("~YUVProgramContext");
    glDisableVertexAttribArray(mPositionLocation);
    glDisableVertexAttribArray(mTexCoordLocation);
    glDetachShader(mProgram, mVertShader);
    glDetachShader(mProgram, mFragmentShader);
    glDeleteShader(mVertShader);
    glDeleteShader(mFragmentShader);
    glDeleteProgram(mProgram);
    glDeleteTextures(3, mYUVTextures);
}

int YUVProgramContext::initProgram() {
    AF_LOGD("createProgram ");
    mProgram = glCreateProgram();
    int mInitRet = compileShader(&mVertShader, YUV_VERTEX_SHADER, GL_VERTEX_SHADER);

    if (mInitRet != 0) {
        AF_LOGE("compileShader mVertShader failed. ret = %d ", mInitRet);
        return mInitRet;
    }

    mInitRet = compileShader(&mFragmentShader, YUV_FRAGMENT_SHADER, GL_FRAGMENT_SHADER);

    if (mInitRet != 0) {
        AF_LOGE("compileShader mFragmentShader failed. ret = %d ", mInitRet);
        return mInitRet;
    }

    glAttachShader(mProgram, mVertShader);
    glAttachShader(mProgram, mFragmentShader);
    glLinkProgram(mProgram);
    GLint status;
    glGetProgramiv(mProgram, GL_LINK_STATUS, &status);

    if (status != GL_TRUE) {
        int length = 0;
        GLchar glchar[256] = {0};
        glGetProgramInfoLog(mProgram, 256, &length, glchar);
        AF_LOGW("linkProgram  error is %s \n", glchar);
        return -1;
    }

    glUseProgram(mProgram);
    getShaderLocations();

    glEnableVertexAttribArray(mPositionLocation);
    glEnableVertexAttribArray(mTexCoordLocation);

    createYUVTextures();

    return 0;
}

void YUVProgramContext::updateScale(IVideoRender::Scale scale) {
    if (mScale != scale) {
        mScale = scale;
        mRegionChanged = true;
    }
}

void YUVProgramContext::updateRotate(IVideoRender::Rotate rotate) {
    if (mRotate != rotate) {
        mRotate = rotate;
        mRegionChanged = true;
    }
}

void YUVProgramContext::updateFlip(IVideoRender::Flip flip) {
    if (mFlip != flip) {
        mFlip = flip;
        mCoordsChanged = true;
    }
}

int YUVProgramContext::updateFrame(std::unique_ptr<IAFFrame> &frame) {

    if (mProgram == 0) {
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

        if (mCropRect.left != videoInfo.crop_left || mCropRect.right != videoInfo.crop_right ||
            mCropRect.top != videoInfo.crop_top || mCropRect.bottom != videoInfo.crop_bottom) {
            mCropRect = {videoInfo.crop_left, videoInfo.crop_right,
                         videoInfo.crop_top, videoInfo.crop_bottom};
            mCoordsChanged = true;
        }

        int *lineSize = frame->getLineSize();
        if (lineSize != nullptr && lineSize[0] != mYLineSize) {
            mYLineSize = lineSize[0];
            mCoordsChanged = true;
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

    if(frame == nullptr && !mProjectionChanged && !mRegionChanged && !mCoordsChanged){
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


    glViewport(0, 0, mWindowWidth, mWindowHeight);
    if(mBackgroundColorChanged) {
        float color[4] = {0.0f,0.0f,0.0f,1.0f};
        cicada::convertToGLColor(mBackgroundColor , color);
        glClearColor(color[0], color[1], color[2], color[3]);
        mBackgroundColorChanged = false;
    }
    glClear(GL_COLOR_BUFFER_BIT);

    if (frame != nullptr) {
        fillDataToYUVTextures(frame->getData(), frame->getLineSize(), frame->getInfo().format);
    }

    bindYUVTextures();

    glUniformMatrix4fv(mProjectionLocation, 1, GL_FALSE, (GLfloat *) mUProjection);
    glUniformMatrix3fv(mColorSpaceLocation, 1, GL_FALSE, (GLfloat *) mUColorSpace);
    glUniform3f(mColorRangeLocation, mUColorRange[0], mUColorRange[1], mUColorRange[2]);
    glVertexAttribPointer(mPositionLocation, 2, GL_FLOAT, GL_FALSE, 0, mDrawRegion);
    glVertexAttribPointer(mTexCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, mFlipCoords);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    return 0;
}


void YUVProgramContext::updateWindowSize(int width, int height, bool windowChanged) {
    if (mWindowWidth == width && mWindowHeight == height && !windowChanged) {
        return;
    }

    mWindowWidth = width;
    mWindowHeight = height;

    mProjectionChanged = true;
    mRegionChanged = true;
}

void YUVProgramContext::updateUProjection() {
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

void YUVProgramContext::updateDrawRegion() {
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


void YUVProgramContext::updateFlipCoords() {


    float leftCropPercent = 0.0f;
    float rightCropPercent = 0.0f;
    float topCropPercent = 0.0f;
    float bottomCropPercent = 0.0f;

    if (mFrameWidth != 0) {
        leftCropPercent = mCropRect.left * 1.0f / mFrameWidth;
        rightCropPercent = mCropRect.right * 1.0f / mFrameWidth;
    }

    if (mFrameHeight != 0) {
        topCropPercent = mCropRect.top * 1.0f / mFrameHeight;
        bottomCropPercent = mCropRect.bottom * 1.0f / mFrameHeight;
    }

    float leftX = 0.0f + leftCropPercent;
    // crop the extra data when draw.
    float rightX = 1.0f - rightCropPercent - (mYLineSize - mFrameWidth) *1.0f / mFrameWidth;
    float topY = 1.0f - topCropPercent;
    float bottomY = 0.0f + bottomCropPercent;

    if (mFlip == IVideoRender::Flip::Flip_Horizontal) {
        mFlipCoords[0] = rightX;
        mFlipCoords[1] = topY;

        mFlipCoords[2] = leftX;
        mFlipCoords[3] = topY;

        mFlipCoords[4] = rightX;
        mFlipCoords[5] = bottomY;

        mFlipCoords[6] = leftX;
        mFlipCoords[7] = bottomY;

    } else if (mFlip == IVideoRender::Flip::Flip_Vertical) {
        mFlipCoords[0] = leftX;
        mFlipCoords[1] = bottomY;

        mFlipCoords[2] = rightX;
        mFlipCoords[3] = bottomY;

        mFlipCoords[4] = leftX;
        mFlipCoords[5] = topY;

        mFlipCoords[6] = rightX;
        mFlipCoords[7] = topY;
    } else if(mFlip == IVideoRender::Flip::Flip_Both){
        mFlipCoords[0] = rightX;
        mFlipCoords[1] = bottomY;

        mFlipCoords[2] = leftX;
        mFlipCoords[3] = bottomY;

        mFlipCoords[4] = rightX;
        mFlipCoords[5] = topY;

        mFlipCoords[6] = leftX;
        mFlipCoords[7] = topY;
    } else  {
        mFlipCoords[0] = leftX;
        mFlipCoords[1] = topY;

        mFlipCoords[2] = rightX;
        mFlipCoords[3] = topY;

        mFlipCoords[4] = leftX;
        mFlipCoords[5] = bottomY;

        mFlipCoords[6] = rightX;
        mFlipCoords[7] = bottomY;
    }
}


void YUVProgramContext::createYUVTextures() {
//    AF_LOGD("createYUVTextures ");

    glDeleteTextures(3, mYUVTextures);
    glGenTextures(3, mYUVTextures);
//Y
    glBindTexture(GL_TEXTURE_2D, mYUVTextures[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//U
    glBindTexture(GL_TEXTURE_2D, mYUVTextures[1]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
//V
    glBindTexture(GL_TEXTURE_2D, mYUVTextures[2]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void YUVProgramContext::fillDataToYUVTextures(uint8_t **data, int *pLineSize, int format) {

    int uvHeight = mFrameHeight;
    if (format == AF_PIX_FMT_YUV422P || format == AF_PIX_FMT_YUVJ422P) {
        uvHeight = mFrameHeight;
    } else if (format == AF_PIX_FMT_YUV420P || format == AF_PIX_FMT_YUVJ420P) {
        uvHeight = mFrameHeight / 2;
    }

    int yWidth = pLineSize[0];
    int uvWidth = yWidth / 2; //uvWidth may not right in some iOS simulators.

    //use linesize to fill data with texture. some android phones which below 4.4 are not performed as excepted.
    // crop the extra data when draw.
//update y
    glBindTexture(GL_TEXTURE_2D, mYUVTextures[0]);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, pLineSize[0] );
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, yWidth, mFrameHeight,
                 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data[0]);
    glUniform1i(mYTexLocation, 0);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

//update u
    glBindTexture(GL_TEXTURE_2D, mYUVTextures[1]);
    glPixelStorei(GL_UNPACK_ROW_LENGTH,  pLineSize[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, uvWidth, uvHeight ,
                 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data[1]);
    glUniform1i(mUTexLocation, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

//update v
    glBindTexture(GL_TEXTURE_2D, mYUVTextures[2]);
    glPixelStorei(GL_UNPACK_ROW_LENGTH,  pLineSize[2]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, uvWidth, uvHeight ,
                 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data[2]);
    glUniform1i(mVTexLocation, 2);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

void YUVProgramContext::bindYUVTextures() {
//    AF_LOGD("bindYUVTextures");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mYUVTextures[0]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, mYUVTextures[1]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, mYUVTextures[2]);
}

void YUVProgramContext::updateColorRange() {
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

void YUVProgramContext::updateColorSpace() {
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

void YUVProgramContext::updateBackgroundColor(uint32_t color) {
    if(color != mBackgroundColor) {
        mBackgroundColorChanged = true;
        mBackgroundColor = color;
    }
}

void YUVProgramContext::getShaderLocations() {
     mProjectionLocation = glGetUniformLocation(mProgram, "u_projection");
     mColorSpaceLocation = glGetUniformLocation(mProgram, "uColorSpace");
     mColorRangeLocation = glGetUniformLocation(mProgram, "uColorRange");
     mPositionLocation = static_cast<GLuint>(glGetAttribLocation(mProgram, "a_position"));
     mTexCoordLocation = static_cast<GLuint>(glGetAttribLocation(mProgram, "a_texCoord"));
     mYTexLocation = glGetUniformLocation(mProgram, "y_tex");
     mUTexLocation = glGetUniformLocation(mProgram, "u_tex");
     mVTexLocation = glGetUniformLocation(mProgram, "v_tex");
}
