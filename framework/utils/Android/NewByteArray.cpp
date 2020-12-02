//
// Created by SuperMan on 2020/10/19.
//

#include "NewByteArray.h"
#include <utils/frame_work_log.h>
#include "JniException.h"


NewByteArray::NewByteArray(JNIEnv *pEnv, const void *source, int len)
{
    if (source == nullptr || pEnv == nullptr) {
        mResult = nullptr;
        mEnv = nullptr;
    } else {
        mEnv = pEnv;
        mResult = pEnv->NewByteArray(len);
        pEnv->SetByteArrayRegion(mResult, 0, len, (jbyte *) (source));
        JniException::clearException(mEnv);
    }
}

NewByteArray::~NewByteArray()
{
    if (mResult != nullptr) {
        mEnv->DeleteLocalRef(mResult);
        JniException::clearException(mEnv);
    }

    mResult = nullptr;
}

jbyteArray NewByteArray::getArray()
{
    return mResult;
}