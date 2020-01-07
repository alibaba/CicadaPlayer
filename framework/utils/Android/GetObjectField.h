//
// Created by lifujun on 2019/4/2.
//

#ifndef SOURCE_GETOBJECTFIELD_H
#define SOURCE_GETOBJECTFIELD_H

#include <jni.h>

class GetObjectField {
public:
    GetObjectField(JNIEnv *pEnv, jobject obj, jfieldID fieldID);

    ~GetObjectField();

    jobject getObjectField();

private:
    JNIEnv *mEnv;
    jobject mResult;

private:
    GetObjectField(GetObjectField &)
    {

    }

    const GetObjectField &operator=(const GetObjectField &);

};


#endif //SOURCE_GETOBJECTFIELD_H
