//
// Created by lifujun on 2019/1/9.
//

#include "FindClass.h"
#include "JniException.h"

FindClass::FindClass(JNIEnv *pEnv, const char *className)
{
    if (className == nullptr || pEnv == nullptr) {
        mResult = nullptr;
        mEnv    = nullptr;
    } else {
        mEnv    = pEnv;
        mResult = pEnv->FindClass(className);
        JniException::clearException(mEnv);
    }
}

FindClass::~FindClass()
{
    if (mResult != nullptr) {
        mEnv->DeleteLocalRef(mResult);
        JniException::clearException(mEnv);
    }

    mResult = nullptr;
}

jclass FindClass::getClass()
{
    return mResult;
}