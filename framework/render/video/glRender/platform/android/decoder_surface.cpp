#define LOG_TAG "DecoderSurface"

#include "decoder_surface.h"
#include <utils/Android/AndroidJniHandle.h>
#include <utils/frame_work_log.h>
#include <utils/Android/JniEnv.h>
#include <utils/Android/JniException.h>

namespace Cicada {
    jclass gj_ds_Class = nullptr;
    jmethodID gj_ds_UpdateTexImgMethod = nullptr;
    jmethodID gj_ds_GetTransformMatrix = nullptr;
    jmethodID gj_ds_midSurfaceTexture = nullptr;
    jmethodID gj_ds_midCreateSurface = nullptr;
    jmethodID gj_ds_Dispose = nullptr;

    JNIEXPORT void JNICALL jniOnFrameAvailable
            (JNIEnv *env, jclass jc, jlong handler)
    {
        DecoderSurface *decoderSurface = (DecoderSurface *)handler;
        decoderSurface->onFrameAvailable();
    }

    void DecoderSurface::init(JNIEnv *env) {
        if (gj_ds_Class == nullptr) {
            jclass lc = env->FindClass("com/cicada/player/utils/DecoderSurfaceTexture");

            if (lc) {
                gj_ds_Class = reinterpret_cast<jclass>(env->NewGlobalRef(lc));
            } else {
                JniException::clearException(env);
            }

            env->DeleteLocalRef(lc);
        }

        if (!gj_ds_Class) {
            AF_LOGE("Could not find surfaceTextureClass") ;
            return ;
        }

        gj_ds_midSurfaceTexture = env->GetMethodID(gj_ds_Class,
                                                   "<init>", "()V");
        gj_ds_midCreateSurface = env->GetMethodID(gj_ds_Class,
                                                      "createSurface", "(IJ)Landroid/view/Surface;");
        gj_ds_UpdateTexImgMethod = env->GetMethodID(gj_ds_Class,
                                                  "updateTexImage", "()V");
        gj_ds_GetTransformMatrix = env->GetMethodID(gj_ds_Class,
                                                  "getTransformMatrix", "([F)V");
        gj_ds_Dispose = env->GetMethodID(gj_ds_Class, "dispose", "()V");

        //create surfacetexture
        JNINativeMethod ClsMethods[] = {
                { "onFrameAvailable", "(J)V", (void *)jniOnFrameAvailable },
        };

        if (env->RegisterNatives(gj_ds_Class, ClsMethods, 1) < 0) {
            AF_LOGE("fail to register native methods");
        }

    }

    void DecoderSurface::unInit(JNIEnv *env) {
        if(gj_ds_Class != nullptr){
            env->DeleteGlobalRef(gj_ds_Class);
            gj_ds_Class = nullptr;
        }
    }


    DecoderSurface::~DecoderSurface()
    {
        JniEnv  jniEnv;
        JNIEnv* handle = jniEnv.getEnv();

        if (handle) {
            if (mbNeedReleaseSurface) {
                handle->DeleteGlobalRef(mSurface);
                mSurface = nullptr;
                mbNeedReleaseSurface = false;
            }

            if (mSurfaceTexture) {
                // stop SurfaceTexture Thread
                handle->CallVoidMethod(mSurfaceTexture, gj_ds_Dispose);
                // delete instance
                handle->DeleteGlobalRef(mSurfaceTexture);
            }
        }
    }

    int DecoderSurface::Init(int textureId, jobject surfaceObj)
    {
        if (surfaceObj) {
            mSurface = surfaceObj;
            return 0;
        }

        JniEnv  jniEnv;
        JNIEnv* handle = jniEnv.getEnv();

        if (!handle) {
            AF_LOGE("jni attach failed.");
            return -1;
        }

        mTextureId = textureId;

        if (textureId >= 0 && createSurface(textureId, (JNIEnv *)handle) != 0) {
            AF_LOGE("faild tod create surface");
            return -2;
        }

        return 0;
    }

    int DecoderSurface::GetTexutreId()
    {
        return mTextureId;
    }

    jobject DecoderSurface::GetSurface()
    {
        return mSurface;
    }

    bool DecoderSurface::UpdateTexImg()
    {
        if (!mSurfaceTexture) {
            AF_LOGE("mSurfaceTexture is nullptr") ;
            return false;
        }

        JniEnv  jniEnv;
        JNIEnv* handle = jniEnv.getEnv();

        if (handle) {
            handle->CallVoidMethod(mSurfaceTexture, gj_ds_UpdateTexImgMethod);
        }

        return true;
    }

    bool DecoderSurface::GetTransformMatrix(float matrix[16])
    {
        bool ret = false;

        if (!mSurfaceTexture) {
            AF_LOGE("mSurfaceTexture is nullptr") ;
            return false;
        }

        JniEnv  jniEnv;
        JNIEnv* handle = jniEnv.getEnv();

        if (handle) {
            AndroidJniHandle<jfloatArray> arrMatrix(handle->NewFloatArray(16));
            handle->CallVoidMethod(mSurfaceTexture, gj_ds_GetTransformMatrix, (jfloatArray)arrMatrix);

            if (arrMatrix != nullptr) {
                jfloat *matrix_get = handle->GetFloatArrayElements(arrMatrix, JNI_FALSE);

                for (int i = 0; i < 16; ++i) {
                    matrix[i] = matrix_get[i];
                }

                handle->ReleaseFloatArrayElements(arrMatrix, matrix_get, 0);
                ret = true;
            }
        }

        return ret;
    }

    int DecoderSurface::createSurface(int textureId, JNIEnv *env)
    {

        AndroidJniHandle<jobject> obj(env->NewObject(gj_ds_Class, gj_ds_midSurfaceTexture));

        if (!obj) {
            AF_LOGE("failed to create surfaceTexture obj") ;
            return -4;
        }

        mSurfaceTexture = env->NewGlobalRef(obj);

        if (!mSurfaceTexture) {
            AF_LOGE("failed to create surfaceTexture") ;
            return -5;
        }

        obj = (jobject)env->CallObjectMethod(mSurfaceTexture, gj_ds_midCreateSurface, mTextureId, (long)this);

        if (!obj) {
            AF_LOGE("failed to create Surface obj") ;
            return -7;
        }

        mSurface = env->NewGlobalRef(obj);

        if (!mSurface) {
            AF_LOGE("failed to create Surface") ;
            return -8;
        }

        mbNeedReleaseSurface = true;
        return 0;
    }

    void DecoderSurface::onFrameAvailable()
    {
        mCallback->onFrameAvailable();
//        UpdateTexImg();
    }


}
