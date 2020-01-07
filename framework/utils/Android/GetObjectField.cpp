//
// Created by lifujun on 2019/4/2.
//

#include <utils/frame_work_log.h>
#include "GetObjectField.h"
#include "JniException.h"


GetObjectField::GetObjectField(JNIEnv *pEnv, jobject obj, jfieldID fieldID)
{
    if (obj == nullptr || pEnv == nullptr || fieldID == nullptr) {
        mResult = nullptr;
        mEnv = nullptr;
    } else {
        mEnv = pEnv;
        mResult = pEnv->GetObjectField(obj, fieldID);
        JniException::clearException(mEnv);
    }
}

GetObjectField::~GetObjectField()
{
    if (mResult != nullptr) {
        mEnv->DeleteLocalRef(mResult);
        JniException::clearException(mEnv);
    }

    mResult = nullptr;
}

jobject GetObjectField::getObjectField()
{
    return mResult;
}