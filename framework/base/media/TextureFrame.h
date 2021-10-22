//
// Created by SuperMan on 8/24/21.
//

#ifndef SOURCE_TEXTUREFRAME_H
#define SOURCE_TEXTUREFRAME_H


#include "IAFPacket.h"

class CICADA_CPLUS_EXTERN TextureFrame : public IAFFrame {
public:
    enum TextureType {
        TEXTURE_YUV,
        TEXTURE_RGBA,
    };

    TextureFrame(TextureType textureType, void *glContext, int *texture, int *linesize, int width, int height);

    ~TextureFrame();

    uint8_t **getData() override
    {
        return nullptr;
    }

    int *getLineSize() override
    {
        return mLineSize;
    }

    FrameType getType() override
    {
        return FrameTypeVideo;
    }

    std::unique_ptr<IAFFrame> clone() override
    {
        return nullptr;
    }

    int *getTexture()
    {
        return mTexture;
    }

    void *getGlContext()
    {
        return mGlContext;
    }

    TextureType getTextureType()
    {
        return mTextureType;
    }

private:
    TextureType mTextureType;
    void *mGlContext;
    int *mTexture{nullptr};
    int *mLineSize{nullptr};
};


#endif//SOURCE_TEXTUREFRAME_H
