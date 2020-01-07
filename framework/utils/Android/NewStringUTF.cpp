//
// Created by lifujun on 2019/1/9.
//

#include "NewStringUTF.h"
#include "JniException.h"


NewStringUTF::NewStringUTF(JNIEnv *pEnv, const char *source)
{
    if (source == nullptr || pEnv == nullptr) {
        mResult = nullptr;
        mEnv    = nullptr;
    } else {
        mEnv    = pEnv;
        mResult = pEnv->NewStringUTF(source);
        JniException::clearException(mEnv);
    }
}


NewStringUTF::~NewStringUTF()
{
    if (mResult != nullptr) {
        mEnv->DeleteLocalRef(mResult);
        JniException::clearException(mEnv);
    }

    mResult = nullptr;
}

jstring NewStringUTF::getString()
{
    return mResult;
}
