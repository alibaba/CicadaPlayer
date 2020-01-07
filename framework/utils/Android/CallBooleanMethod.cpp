//
// Created by lifujun on 2019/1/13.
//

#include "CallBooleanMethod.h"
#include "JniException.h"


CallBooleanMethod::CallBooleanMethod(JNIEnv *pEnv, jobject obj, jmethodID methodId)
{
    if (obj == nullptr || methodId == nullptr) {
        mResult = static_cast<jboolean>(false);
        mEnv    = nullptr;
    } else {
        mEnv    = pEnv;
        mResult = pEnv->CallBooleanMethod(obj, methodId);
        JniException::clearException(pEnv);
    }
}

CallBooleanMethod::~CallBooleanMethod()
{
}

jboolean CallBooleanMethod::getValue()
{
    return mResult;
}