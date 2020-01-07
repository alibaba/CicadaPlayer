//
// Created by lifujun on 2019/1/9.
//

#include "GetObjectClass.h"
#include "JniException.h"

GetObjectClass::GetObjectClass(JNIEnv *pEnv, jobject obj)
{
    if (obj == nullptr || pEnv == nullptr) {
        mResult = nullptr;
        mEnv    = nullptr;
    } else {
        mEnv    = pEnv;
        mResult = pEnv->GetObjectClass(obj);
        JniException::clearException(mEnv);
    }
}

GetObjectClass::~GetObjectClass()
{
    if (mResult != nullptr) {
        mEnv->DeleteLocalRef(mResult);
        JniException::clearException(mEnv);
    }

    mResult = nullptr;
}

jclass GetObjectClass::getClass()
{
    return mResult;
}


