//
// Created by lifujun on 2019/1/9.
//

#include "GetStringUTFChars.h"

#include "JniException.h"

GetStringUTFChars::GetStringUTFChars(JNIEnv *pEnv, jstring source)
{
    if (source == nullptr || pEnv == nullptr) {
        mEnv    = nullptr;
        mResult = nullptr;
    } else {
        mSource = source;
        mEnv    = pEnv;
        mResult = const_cast<char *>(pEnv->GetStringUTFChars(source, nullptr));
        JniException::clearException(mEnv);
    }
}

GetStringUTFChars::~GetStringUTFChars()
{
    if (mResult != nullptr) {
        mEnv->ReleaseStringUTFChars(mSource, mResult);
        JniException::clearException(mEnv);
    }

    mResult = nullptr;
}

char *GetStringUTFChars::getChars()
{
    return mResult;
}
