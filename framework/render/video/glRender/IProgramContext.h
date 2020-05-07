//
// Created by lifujun on 2019/8/20.
//

#ifndef SOURCE_IPROGRAMCONTEXT_H
#define SOURCE_IPROGRAMCONTEXT_H

#include "render/video/IVideoRender.h"

#if TARGET_PLATFORM == PLATFORM_IOS

#include <OpenGLES/gltypes.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#endif

class IProgramContext {

public:
    static int compileShader(GLuint *shader, const char *src, GLenum type) {
        GLuint shaderId = glCreateShader(type);
        glShaderSource(shaderId, 1, &src, nullptr);
        glCompileShader(shaderId);
        GLint status;
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &status);

        if (status != GL_TRUE) {
            int length = 0;
            GLchar glchar[256] = {0};
            glGetShaderInfoLog(shaderId, 256, &length, glchar);
            glDeleteShader(shaderId);
            return -1;
        }

        *shader = shaderId;
        return 0;
    }

public:
    virtual ~IProgramContext() = default;

    virtual int initProgram() = 0;

    virtual void createSurface() {
    }

    virtual void *getSurface() {
        return nullptr;
    };

    virtual void updateScale(IVideoRender::Scale scale) = 0;

    virtual void updateFlip(IVideoRender::Flip flip) = 0;

    virtual void updateRotate(IVideoRender::Rotate rotate) = 0;

    virtual void updateBackgroundColor(uint32_t color) = 0;

    virtual void updateWindowSize(int width, int height, bool windowChanged) = 0;

    virtual int updateFrame(std::unique_ptr<IAFFrame> &frame) = 0;

};


#endif //SOURCE_IPROGRAMCONTEXT_H
