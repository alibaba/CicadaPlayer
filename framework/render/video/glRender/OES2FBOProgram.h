//
// Created by SuperMan on 8/20/21.
//

#ifndef SOURCE_OES2FBOPROGRAM_H
#define SOURCE_OES2FBOPROGRAM_H


#include <GLES2/gl2.h>

class OES2FBOProgram {

public:
    OES2FBOProgram();

    ~OES2FBOProgram();

    int initProgram();

    bool updateFrameBuffer(int width, int height);

    GLuint getFrameBuffer();

    GLuint getFrameTexture();

    void useProgram();

    void enableDrawRegion(GLfloat drawRegion[12]);

    void enableFlipCoords(GLfloat flipCoords[8]);

    void disableDrawRegion();

    void disableFlipCoords();

    void uniform1i();

private:
    void destroyFrameBuffer();

private:
    int mInitRet = 0;

    GLuint mDisProgram = {0};
    GLuint mDisVertShader{0};
    GLuint mDisFragmentShader{0};
    GLuint mDisPositionLocation{0};
    GLuint mDisTexCoordLocation{0};
    GLint mDisTextureLocation{0};
    GLuint *mFrameBuffers{nullptr};
    GLuint *mFrameBufferTextures{nullptr};

    int mFrameWidth{0};
    int mFrameHeight{0};
};


#endif//SOURCE_OES2FBOPROGRAM_H
