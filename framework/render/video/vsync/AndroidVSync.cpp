//
// Created by lifujun on 2019/9/6.
//

#define  LOG_TAG "AndroidVSync"

#include <jni.h>
#include <utils/frame_work_log.h>
#include <utils/Android/JniEnv.h>
#include <utils/Android/JniException.h>
#include "AndroidVSync.h"

jclass    gj_VsyncTimerClass = nullptr;
jmethodID gj_VsyncTimer_init = nullptr;
jmethodID gj_VsyncTimer_start = nullptr;
jmethodID gj_VsyncTimer_pause = nullptr;
jmethodID gj_VsyncTimer_destroy = nullptr;

int64_t AndroidVSync::mPeriod = 0;
float AndroidVSync::mHz = 0;

void AndroidVSync::init(JNIEnv *env) {

    if (!env) {
        AF_LOGE("jni attach failed.");
        return;
    }

    if (gj_VsyncTimerClass == nullptr) {
        jclass lc = env->FindClass("com/cicada/player/utils/VsyncTimer");
        if (lc) {
            gj_VsyncTimerClass = reinterpret_cast<jclass>(env->NewGlobalRef(lc));
        } else {
            JniException::clearException(env);
        }
        env->DeleteLocalRef(lc);
    }

    gj_VsyncTimer_init = env->GetMethodID(gj_VsyncTimerClass,
                                             "<init>",
                                             "(J)V");
    gj_VsyncTimer_start = env->GetMethodID(gj_VsyncTimerClass,
                                              "start",
                                              "()V");
    gj_VsyncTimer_pause = env->GetMethodID(gj_VsyncTimerClass,
                                              "pause",
                                              "()V");
    gj_VsyncTimer_destroy = env->GetMethodID(gj_VsyncTimerClass,
                                                "destroy",
                                                "()V");
    JNINativeMethod ClsMethods[] = {
            {"onInit",    "(J)I",  (void *) OnInit},
            {"onVsync",   "(JJ)I", (void *) OnVsync},
            {"onDestroy", "(J)V",  (void *) OnDestroy},
    };

    if (env->RegisterNatives(gj_VsyncTimerClass, ClsMethods,
                                sizeof(ClsMethods) / sizeof(JNINativeMethod)) < 0) {
        AF_LOGE("fail to register native methods");
        return;
    }
}

void AndroidVSync::unInit(JNIEnv *env) {
    if(gj_VsyncTimerClass != nullptr){
        env->DeleteGlobalRef(gj_VsyncTimerClass);
        gj_VsyncTimerClass = nullptr;
    }
}

AndroidVSync::AndroidVSync(IVSync::Listener &listener) : IVSync(listener)
{

}

AndroidVSync::~AndroidVSync()
{
    if (mVsyncTimer == nullptr) {
        return;
    }

    JniEnv  jniEnv;
    JNIEnv* handle = jniEnv.getEnv();

    if (!handle) {
        AF_LOGE("jni attach failed.");
        return;
    }

    handle->CallVoidMethod(mVsyncTimer, gj_VsyncTimer_destroy);
}

void AndroidVSync::start()
{
    JniEnv  jniEnv;
    JNIEnv* handle = jniEnv.getEnv();

    if (!handle) {
        AF_LOGE("jni attach failed.");
        return;
    }

    if (mVsyncTimer == nullptr) {
        long ptrL = reinterpret_cast<long>(this);
        jlong ptr = (jlong) this;
        jobject newObject = handle->NewObject(gj_VsyncTimerClass, gj_VsyncTimer_init,
                                              ptr);
        mVsyncTimer = handle->NewGlobalRef(newObject);
    }

    handle->CallVoidMethod(mVsyncTimer, gj_VsyncTimer_start);
}

void AndroidVSync::pause()
{
    if (mVsyncTimer == nullptr) {
        return;
    }

    JniEnv  jniEnv;
    JNIEnv* handle = jniEnv.getEnv();

    if (!handle) {
        AF_LOGE("jni attach failed.");
        return;
    }

    handle->CallVoidMethod(mVsyncTimer, gj_VsyncTimer_pause);
}

int AndroidVSync::OnInit(JNIEnv *env, jclass jc, jlong ptr)
{
    auto *vysncTimer = reinterpret_cast<AndroidVSync *>((long) ptr);

    if (vysncTimer != nullptr) {
        return vysncTimer->mListener.VSyncOnInit();
    }

    return 0;
}

int AndroidVSync::OnVsync(JNIEnv *env, jclass jc, jlong ptr, jlong nanos)
{
    auto *pVSync = reinterpret_cast<AndroidVSync *>((long) ptr);
    int64_t time = nanos / 1000;

    if (mPeriod == 0) {
        static  int64_t lastUsos = INT64_MIN;

        if (lastUsos == INT64_MIN) {
            lastUsos = time;
        } else {
            mPeriod = static_cast<int>(time - lastUsos);

            if (mPeriod < 1000000 / 75) {
                mHz = 90;
            } else {
                mHz = 60;
            }

            mPeriod = static_cast<int64_t>(1000000 / mHz);
        }
    }

    if (pVSync != nullptr) {
        return pVSync->mListener.onVSync(mPeriod ? time / mPeriod : 0);
    }

    return 0;
}

void AndroidVSync::OnDestroy(JNIEnv *env, jclass jc, jlong ptr)
{
    auto *vysncTimer = reinterpret_cast<AndroidVSync *>((long) ptr);

    if (vysncTimer != nullptr) {
        vysncTimer->mListener.VSyncOnDestroy();
        env->DeleteGlobalRef(vysncTimer->mVsyncTimer);
    }
}

