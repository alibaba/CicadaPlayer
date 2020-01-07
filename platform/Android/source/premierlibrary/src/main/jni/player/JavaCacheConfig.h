//
// Created by lifujun on 2019/5/23.
//

#ifndef SOURCE_JAVACACHECONFIG_H
#define SOURCE_JAVACACHECONFIG_H


#include <jni.h>
#include <cacheModule/cache/CacheConfig.h>

class JavaCacheConfig {
public:
    static void init(JNIEnv *pEnv);

    static void unInit(JNIEnv *pEnv);

    static void convertTo(CacheConfig& config, JNIEnv *mEnv, jobject pJobject);

};


#endif //SOURCE_JAVACACHECONFIG_H
