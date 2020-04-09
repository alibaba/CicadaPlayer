#include <jni.h>

#define LOG_TAG "JNI"
#define NOLOGD

#include <render/video/vsync/AndroidVSync.h>
#include <render/video/glRender/platform/android/decoder_surface.h>
#include <utils/Android/JniEnv.h>
#include "utils/JavaLogger.h"
#include "player/JavaPlayerConfig.h"
#include "player/JavaCacheConfig.h"
#include "player/JavaMediaInfo.h"
#include "player/JavaTrackInfo.h"
#include "player/JavaGlobalSettings.h"
#include "player/NativeBase.h"

using namespace Cicada;

int initJavaInfo(JNIEnv *env)
{
    JavaLogger::init(env);
    JavaCacheConfig::init(env);
    JavaMediaInfo::init(env);
    JavaTrackInfo::init(env);
    NativeBase::init(env);
    JavaPlayerConfig::init(env);
    AndroidVSync::init(env);
    DecoderSurface::init(env);
    JavaGlobalSettings::init(env);
    int result = NativeBase::registerMethod(env);

    if (result == JNI_FALSE) {
        return JNI_FALSE;
    }

    result = JavaLogger::registerMethod(env);

    if (result == JNI_FALSE) {
        return JNI_FALSE;
    }

    result = JavaGlobalSettings::registerMethod(env);

    if (result == JNI_FALSE) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

void unInitJavaInfo(JNIEnv *env)
{
    JavaCacheConfig::unInit(env);
    JavaMediaInfo::unInit(env);
    JavaTrackInfo::unInit(env);
    NativeBase::unInit(env);
    JavaPlayerConfig::unInit(env);
    AndroidVSync::unInit(env);
    DecoderSurface::unInit(env);
    JavaLogger::unInit(env);
    JavaGlobalSettings::unInit(env);
}

extern "C"
JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    AF_LOGE("0328 JNI_OnLoad");
    JniEnv::init(vm);
    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();
    int result = initJavaInfo(mEnv);

    if (result == JNI_FALSE) {
        return JNI_FALSE;
    }

    return JNI_VERSION_1_4;
}


extern "C"
JNIEXPORT void JNI_OnUnload(JavaVM *vm, void *reserved)
{
    AF_LOGE("0328 JNI_OnUnload");
    JniEnv Jenv;
    JNIEnv *env = Jenv.getEnv();
    unInitJavaInfo(env);
}
