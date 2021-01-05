//
// Created by lifujun on 2019/1/9.
//

#ifndef PLAYERSDKDEMO_JNISTRING_H
#define PLAYERSDKDEMO_JNISTRING_H


#include <jni.h>
#include <utils/CicadaType.h>

class CICADA_CPLUS_EXTERN NewStringUTF {

public:
    NewStringUTF(JNIEnv *pEnv, const char *source);

    ~NewStringUTF();

public:
    jstring getString();

private:
    JNIEnv *mEnv;
    jstring mResult;


private:
    NewStringUTF(NewStringUTF &)
    {

    }

    const NewStringUTF &operator=(const NewStringUTF &);

private:
    static char checkUtfBytes(const char *bytes, const char **errorKind);
};


#endif //PLAYERSDKDEMO_JNISTRING_H
