//
// Created by lifujun on 2019/1/13.
//

#ifndef SOURCE_CALLBOOLEANMETHOD_H
#define SOURCE_CALLBOOLEANMETHOD_H


#include <jni.h>

class CallBooleanMethod {
public:
    CallBooleanMethod(JNIEnv *pEnv, jobject obj, jmethodID methodId);

    ~CallBooleanMethod();

    jboolean getValue();

private:
    JNIEnv  *mEnv;
    jboolean mResult;

private:
    CallBooleanMethod(CallBooleanMethod &)
    {

    }

    const CallBooleanMethod &operator=(const CallBooleanMethod &);
};



#endif //SOURCE_CALLBOOLEANMETHOD_H
