//
// Created by lifujun on 2019/1/9.
//

#ifndef SOURCE_GETSTRINGUTFCHARS_H
#define SOURCE_GETSTRINGUTFCHARS_H


#include <jni.h>
#include <utils/CicadaType.h>

class CICADA_CPLUS_EXTERN GetStringUTFChars {

public:
    GetStringUTFChars(JNIEnv *pEnv, jstring source);

    ~GetStringUTFChars();

public:
    char *getChars();

private:
    char    *mResult;
    jstring mSource;
    JNIEnv  *mEnv;

private:
    GetStringUTFChars(GetStringUTFChars &)
    {

    }

    const GetStringUTFChars &operator=(const GetStringUTFChars &);

};


#endif //SOURCE_GETSTRINGUTFCHARS_H
