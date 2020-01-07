//
// Created by lifujun on 2018/12/7.
//

#ifndef SOURCE_JAVAPLAYERCONFIG_H
#define SOURCE_JAVAPLAYERCONFIG_H


#include <jni.h>
#include <cstddef>
#include <MediaPlayerConfig.h>

using namespace Cicada;

class JavaPlayerConfig {
public:
    static void init(JNIEnv *pEnv);

    static void unInit(JNIEnv *pEnv);

    static jobject getJPlayerConfig(JNIEnv *mEnv, const MediaPlayerConfig *playerConfig);

    static MediaPlayerConfig convertTo(JNIEnv *mEnv , jobject config);
};


#endif //SOURCE_JAVAPLAYERCONFIG_H
