//
// Created by lifujun on 2019/6/3.
//

#ifndef SOURCE_CALLINTEGERMETHOD_H
#define SOURCE_CALLINTEGERMETHOD_H

#include <jni.h>

class CallIntMethod {
public:
    CallIntMethod(JNIEnv *pEnv, jobject obj, jmethodID methodId);

    ~CallIntMethod();

    jint getValue();

private:
    JNIEnv  *mEnv;
    jint mResult;

private:
    CallIntMethod(CallIntMethod &)
    {

    }

    const CallIntMethod &operator=(const CallIntMethod &);
};


#endif //SOURCE_CALLINTEGERMETHOD_H
