//
// Created by SuperMan on 8/24/21.
//

#include "TextureFrame.h"
#include <cassert>

TextureFrame::TextureFrame(TextureFrame::TextureType textureType, void *glContext, int *texture, int *linesize, int width, int height)
{

    mInfo.video.format = AF_PIX_FMT_CICADA_TEXTURE;

    int byteCount = 0;
    if (textureType == TEXTURE_RGBA) {
        byteCount = 1 * sizeof(int);
    } else if (textureType == TEXTURE_YUV) {
        byteCount = 3 * sizeof(int);
    } else {
        assert(0);
    }
    mTextureType = textureType;

    if (linesize != nullptr) {
        mLineSize = static_cast<int *>(malloc(byteCount));
        memcpy(mLineSize, linesize, byteCount);
    }

    if (texture != nullptr) {
        mTexture = static_cast<int *>(malloc(byteCount));
        memcpy(mTexture, texture, byteCount);
    }

    mGlContext = glContext;
    mInfo.video.width = width;
    mInfo.video.height = height;
}

TextureFrame::~TextureFrame()
{
    free(mLineSize);
    free(mTexture);
}
