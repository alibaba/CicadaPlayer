//
// Created by SuperMan on 2020/10/19.
//

#ifndef SOURCE_NEWBYTEARRAY_H
#define SOURCE_NEWBYTEARRAY_H

#include <jni.h>

class NewByteArray {
public:
    NewByteArray(JNIEnv *pEnv, const void*source , int len);

    ~NewByteArray();

public:
    jbyteArray getArray();

private:
    JNIEnv *mEnv;
    jbyteArray mResult;


private:
    NewByteArray(NewByteArray &)
    {

    }

    const NewByteArray &operator=(const NewByteArray &);

};



#endif //SOURCE_NEWBYTEARRAY_H
