//
// Created by lifujun on 2019/1/9.
//

#ifndef PLAYERSDKDEMO_CALLOBJECTMETHOD_H
#define PLAYERSDKDEMO_CALLOBJECTMETHOD_H


#include <jni.h>

class CallObjectMethod {
public:
    CallObjectMethod(JNIEnv *pEnv, jobject obj, jmethodID methodId);

    ~CallObjectMethod();

    jobject getValue();

private:
    JNIEnv  *mEnv;
    jobject mResult;

private:
    CallObjectMethod(CallObjectMethod &)
    {

    }

    const CallObjectMethod &operator=(const CallObjectMethod &);
};


#endif //PLAYERSDKDEMO_CALLOBJECTMETHOD_H
