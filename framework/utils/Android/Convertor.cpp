//
// Created by lifujun on 2019/3/29.
//

#include <cstring>
#include <malloc.h>
#include "Convertor.h"
#include "JniException.h"


char *Convertor::jByteArrayToChars(JNIEnv *env, jbyteArray bytearray)
{
    jbyte *bytes    = env->GetByteArrayElements(bytearray, 0);
    int   chars_len = env->GetArrayLength(bytearray);
    char *chars = static_cast<char *>(malloc(chars_len));
    memcpy(chars, bytes, chars_len);
    env->ReleaseByteArrayElements(bytearray, bytes, 0);
    JniException::clearException(env);
    return chars;
}

char *Convertor::jByteArrayToChars_New(JNIEnv *env, jbyteArray bytearray)
{
    jbyte *bytes    = env->GetByteArrayElements(bytearray, 0);
    int   chars_len = env->GetArrayLength(bytearray);
    char *chars = new char[chars_len + 1]();
    memcpy(chars, bytes, chars_len);
    env->ReleaseByteArrayElements(bytearray, bytes, 0);
    JniException::clearException(env);
    return chars;
}