//
// Created by lifujun on 2019/1/9.
//

#ifndef PLAYERSDKDEMO_JNICLASS_H
#define PLAYERSDKDEMO_JNICLASS_H


#include <jni.h>

class GetObjectClass {

public:
    GetObjectClass(JNIEnv *pEnv, jobject pJobject);

    ~GetObjectClass();

    jclass getClass();

private:
    jclass mResult;
    JNIEnv *mEnv;


private:
    GetObjectClass(GetObjectClass &)
    {

    }

    const GetObjectClass &operator=(const GetObjectClass &);
};


#endif //PLAYERSDKDEMO_JNICLASS_H
