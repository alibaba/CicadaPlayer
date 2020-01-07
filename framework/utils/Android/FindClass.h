//
// Created by lifujun on 2019/1/9.
//

#ifndef SOURCE_FINDCLASS_H
#define SOURCE_FINDCLASS_H


#include <jni.h>

class FindClass {
public:
    FindClass(JNIEnv *pEnv, const char *className);

    ~FindClass();

    jclass getClass();

private:
    JNIEnv *mEnv;
    jclass mResult;

private:
    FindClass(FindClass &)
    {

    }

    const FindClass &operator=(const FindClass &);

};


#endif //SOURCE_FINDCLASS_H
