//
// Created by lifujun on 2019/9/24.
//

#ifndef SOURCE_JNIENV_H
#define SOURCE_JNIENV_H


#include <jni.h>

class JniEnv {
public:
    JniEnv();

    ~JniEnv();

    JNIEnv *getEnv();


public:
    static void init(JavaVM *vm);

private:
    static void JNI_ThreadDestroyed(void *value);

private:
    JNIEnv *mEnv;

};


#endif //SOURCE_JNIENV_H
