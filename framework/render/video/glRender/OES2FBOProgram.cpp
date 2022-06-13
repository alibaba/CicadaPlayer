//
// Created by SuperMan on 8/20/21.
//
#define LOG_TAG "OES2FBOProgram"

#include "OES2FBOProgram.h"
#include "IProgramContext.h"
#include <utils/frame_work_log.h>

static const char DIS_VERTEX_SHADER[] = R"(
        attribute vec4 aPosition;
        attribute vec4 aTextureCoord;
        varying vec2 textureCoordinate;
        void main() {
            gl_Position = aPosition;
            textureCoordinate = aTextureCoord.xy;
        }
)";

static const char DIS_FRAGMENT_SHADER[] = R"(
        precision mediump float;
        varying vec2 textureCoordinate;
        uniform sampler2D inputTexture;
        void main() {
            gl_FragColor = texture2D(inputTexture, textureCoordinate);
        }
)";

OES2FBOProgram::OES2FBOProgram()
{}

OES2FBOProgram::~OES2FBOProgram()
{
    glDisableVertexAttribArray(mDisPositionLocation);
    glDisableVertexAttribArray(mDisTexCoordLocation);
    glDetachShader(mDisProgram, mDisVertShader);
    glDetachShader(mDisProgram, mDisFragmentShader);
    glDeleteShader(mDisVertShader);
    glDeleteShader(mDisFragmentShader);
    glDeleteProgram(mDisProgram);

    destroyFrameBuffer();
}

int OES2FBOProgram::initProgram()
{
    AF_LOGD("createDisProgram ");
    mDisProgram = glCreateProgram();

    mInitRet = IProgramContext::compileShader(&mDisVertShader, DIS_VERTEX_SHADER, GL_VERTEX_SHADER);

    if (mInitRet != 0) {
        AF_LOGE("compileShader mDisVertShader failed. ret = %d ", mInitRet);
        return mInitRet;
    }

    mInitRet = IProgramContext::compileShader(&mDisFragmentShader, DIS_FRAGMENT_SHADER, GL_FRAGMENT_SHADER);

    if (mInitRet != 0) {
        AF_LOGE("compileShader mDisFragmentShader failed. ret = %d ", mInitRet);
        return mInitRet;
    }

    glAttachShader(mDisProgram, mDisVertShader);
    glAttachShader(mDisProgram, mDisFragmentShader);
    glLinkProgram(mDisProgram);

    GLint status;
    glGetProgramiv(mDisProgram, GL_LINK_STATUS, &status);

    if (status != GL_TRUE) {
        int length = 0;
        GLchar glchar[256] = {0};
        glGetProgramInfoLog(mDisProgram, 256, &length, glchar);
        AF_LOGW("link DisProgram  error is %s \n", glchar);
        mInitRet = -1;
        return mInitRet;
    }
    mDisPositionLocation = static_cast<GLuint>(glGetAttribLocation(mDisProgram, "aPosition"));
    mDisTexCoordLocation = static_cast<GLuint>(glGetAttribLocation(mDisProgram, "aTextureCoord"));
    mDisTextureLocation = glGetUniformLocation(mDisProgram, "inputTexture");
    mInitRet = 0;
    return mInitRet;
}


bool OES2FBOProgram::updateFrameBuffer(int width, int height)
{
    if (mInitRet < 0) {
        return false;
    }

    if (mFrameBuffers != nullptr && (mFrameWidth != width || mFrameHeight != height)) {
        destroyFrameBuffer();
    }

    if (mFrameBuffers == nullptr) {
        mFrameBuffers = new GLuint[1];
        mFrameBufferTextures = new GLuint[1];

        glGenFramebuffers(1, mFrameBuffers);
        glGenTextures(1, mFrameBufferTextures);

        glBindTexture(GL_TEXTURE_2D, mFrameBufferTextures[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffers[0]);
        glBindTexture(GL_TEXTURE_2D, mFrameBufferTextures[0]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mFrameBufferTextures[0], 0);
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            AF_LOGE("CreateFrameBufferObjs::glCheckFramebufferStatus status[%x] != GL_FRAMEBUFFER_COMPLETE", status);
            return false;
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    return true;
}

GLuint OES2FBOProgram::getFrameBuffer()
{
    return mFrameBuffers[0];
}

GLuint OES2FBOProgram::getFrameTexture()
{
    return mFrameBufferTextures[0];
}

void OES2FBOProgram::destroyFrameBuffer()
{
    if (mFrameBufferTextures != nullptr) {
        glDeleteTextures(1, mFrameBufferTextures);
        delete[] mFrameBufferTextures;
        mFrameBufferTextures = nullptr;
    }

    if (mFrameBuffers != nullptr) {
        glDeleteFramebuffers(1, mFrameBuffers);
        delete[] mFrameBuffers;
        mFrameBuffers = nullptr;
        mFrameBuffers = nullptr;
    }
}

void OES2FBOProgram::enableFlipCoords(GLfloat *flipCoords)
{
    glVertexAttribPointer(mDisTexCoordLocation, 2, GL_FLOAT, GL_FALSE, 8, flipCoords);
    glEnableVertexAttribArray(mDisTexCoordLocation);
}

void OES2FBOProgram::enableDrawRegion(GLfloat *drawRegion)
{
    glVertexAttribPointer(mDisPositionLocation, 3, GL_FLOAT, GL_FALSE, 12, drawRegion);
    glEnableVertexAttribArray(mDisPositionLocation);
}

void OES2FBOProgram::disableDrawRegion()
{
    glDisableVertexAttribArray(mDisPositionLocation);
}

void OES2FBOProgram::disableFlipCoords()
{
    glDisableVertexAttribArray(mDisTexCoordLocation);
}

void OES2FBOProgram::useProgram()
{
    glUseProgram(mDisProgram);
}

void OES2FBOProgram::uniform1i()
{
    glUniform1i(mDisTextureLocation, 0);
}
