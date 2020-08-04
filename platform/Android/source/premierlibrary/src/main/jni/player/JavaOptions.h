//
// Created by lifujun on 2020/7/23.
//

#ifndef SOURCE_JAVAOPTIONS_H
#define SOURCE_JAVAOPTIONS_H


#include <jni.h>
#include <base/options.h>

class JavaOptions {

public:
    static void init(JNIEnv *pEnv);

    static void unInit(JNIEnv *pEnv);

    static jobject convertTo(JNIEnv *pEnv, Cicada::options *options);
};


#endif //SOURCE_JAVAOPTIONS_H
