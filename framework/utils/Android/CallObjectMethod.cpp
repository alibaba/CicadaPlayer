//
// Created by lifujun on 2019/1/9.
//

#include "CallObjectMethod.h"

#include "JniException.h"

CallObjectMethod::CallObjectMethod(JNIEnv *pEnv, jobject obj, jmethodID methodId)
{
    if (obj == nullptr || methodId == nullptr) {
        mResult = nullptr;
        mEnv    = nullptr;
    } else {
        mEnv    = pEnv;
        mResult = pEnv->CallObjectMethod(obj, methodId);
        JniException::clearException(pEnv);
    }
}

CallObjectMethod::~CallObjectMethod()
{
    if (mResult != nullptr) {
        mEnv->DeleteLocalRef(mResult);
        JniException::clearException(mEnv);
    }

    mResult = nullptr;
}

jobject CallObjectMethod::getValue()
{
    return mResult;
}
