//
// Created by lifujun on 2019/5/23.
//

#include "JavaCacheConfig.h"

#include <utils/Android/FindClass.h>
#include <utils/Android/GetObjectField.h>
#include <utils/Android/GetStringUTFChars.h>

static const char *cacheConfigPath = "com/cicada/player/nativeclass/CacheConfig";

//CACHE_CONFIG
jclass gj_CacheConfig_class = nullptr;
jfieldID gj_CacheConfig_Enable = nullptr;
jfieldID gj_CacheConfig_Dir = nullptr;
jfieldID gj_CacheConfig_MaxDuration = nullptr;
jfieldID gj_CacheConfig_MaxSize = nullptr;

void JavaCacheConfig::init(JNIEnv *env)
{
    if (gj_CacheConfig_class == nullptr) {
        FindClass cls(env, cacheConfigPath);
        gj_CacheConfig_class = (jclass) env->NewGlobalRef(cls.getClass());
        gj_CacheConfig_Enable = env->GetFieldID(gj_CacheConfig_class,
                                                "mEnable",
                                                "Z");
        gj_CacheConfig_Dir = env->GetFieldID(gj_CacheConfig_class,
                                             "mDir",
                                             "Ljava/lang/String;");
        gj_CacheConfig_MaxDuration = env->GetFieldID(gj_CacheConfig_class,
                                     "mMaxDurationS",
                                     "J");
        gj_CacheConfig_MaxSize = env->GetFieldID(gj_CacheConfig_class,
                                 "mMaxSizeMB",
                                 "I");
    }
}

void JavaCacheConfig::unInit(JNIEnv *pEnv)
{
    if (gj_CacheConfig_class != nullptr) {
        pEnv->DeleteGlobalRef(gj_CacheConfig_class);
        gj_CacheConfig_class = nullptr;
    }
}

void JavaCacheConfig::convertTo(CacheConfig &cacheConfig, JNIEnv *env, jobject config)
{
    if (config == nullptr) {
        return ;
    }

    GetObjectField getDir(env, config, gj_CacheConfig_Dir);
    auto dirStr = static_cast<jstring>(getDir.getObjectField());
    jlong maxDuration = env->GetLongField(config, gj_CacheConfig_MaxDuration);
    jint maxSize = env->GetIntField(config, gj_CacheConfig_MaxSize);
    jboolean enable = env->GetBooleanField(config, gj_CacheConfig_Enable);
    GetStringUTFChars tmpDir(env, dirStr);
    char *dir = tmpDir.getChars();
    cacheConfig.mCacheDir = (dir == nullptr ? "" : std::string(dir));
    cacheConfig.mEnable = (bool) enable;
    cacheConfig.mMaxDurationS = maxDuration;
    cacheConfig.mMaxDirSizeMB = maxSize;
}
