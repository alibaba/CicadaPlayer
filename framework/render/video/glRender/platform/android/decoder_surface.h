#ifndef TEXTURE_SURFACE_H
#define TEXTURE_SURFACE_H
#ifdef __ANDROID__
#include "jni.h"
namespace Cicada
{

class DecoderSurfaceCallback
{
public:
    virtual void onFrameAvailable() = 0;
};

class DecoderSurface
{
public:
    DecoderSurface(DecoderSurfaceCallback* cb)
    {
        mCallback = cb;
    }

    ~DecoderSurface();

public:

    static void init(JNIEnv* env);

    static void unInit(JNIEnv *env);

    int Init(int textureId, jobject surfaceObj);

    int GetTexutreId();

    jobject GetSurface();

    void UnInit();

    //如果是根据纹理创建的，需要调用update 和 transformmatrix。
    bool UpdateTexImg() ;
    //返回矩阵数组不需要释放
    bool GetTransformMatrix(float matrix[16]);

    void onFrameAvailable();

private:
    int createSurface(int textureId, JNIEnv* env);

private:
    int                     mTextureId{-1};
    bool                    mbNeedReleaseSurface{false};
    jobject                 mSurface{nullptr};
    jobject                 mSurfaceTexture{nullptr};
    DecoderSurfaceCallback* mCallback{nullptr};

};

}

#endif //#ifdef __ANDROID__
#endif // TEXTURE_SURFACE_H
