//
// Created by lifujun on 2019/6/3.
//


#include "CallIntMethod.h"
#include "JniException.h"


CallIntMethod::CallIntMethod(JNIEnv *pEnv, jobject obj, jmethodID methodId)
{
    if (obj == nullptr || methodId == nullptr) {
        mResult = 0;
        mEnv    = nullptr;
    } else {
        mEnv    = pEnv;
        mResult = pEnv->CallIntMethod(obj, methodId);
        JniException::clearException(pEnv);
    }
}

CallIntMethod::~CallIntMethod()
{
}

jint CallIntMethod::getValue()
{
    return mResult;
}
