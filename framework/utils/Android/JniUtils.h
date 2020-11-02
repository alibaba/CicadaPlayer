//
// Created by lifujun on 2019/3/29.
//

#ifndef SOURCE_JNIUTILS_H
#define SOURCE_JNIUTILS_H


#include <jni.h>
#include <string>
#include <map>

class JniUtils {
public:
    static char *jByteArrayToChars(JNIEnv *env, jbyteArray bytearray);

    static char *jByteArrayToChars_New(JNIEnv *env, jbyteArray bytearray);

    static jobject cmap2Jmap(JNIEnv *env, std::map<std::string, std::string> theMap);

    static std::map<std::string, std::string> jmap2cmap(JNIEnv *env, jobject jobj);

    static std::string callStringMethod(JNIEnv *pEnv, jobject jObj, jmethodID method);
};


#endif //SOURCE_JNIUTILS_H
