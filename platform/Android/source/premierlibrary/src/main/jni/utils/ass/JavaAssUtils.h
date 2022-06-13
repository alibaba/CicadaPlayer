//
// Created by SuperMan on 6/29/21.
//

#ifndef SOURCE_JAVAASSUTILS_H
#define SOURCE_JAVAASSUTILS_H


#include <jni.h>
#include <utils/AssUtils.h>

class JavaAssUtils {
public:
    static void init(JNIEnv *pEnv);

    static void unInit(JNIEnv *pEnv);

    static int registerMethod(JNIEnv *pEnv);

    static jobject java_ParseAssHeader(JNIEnv *pEnv, jclass clz, jstring header);

    static jobject java_ParseAssDialogue(JNIEnv *pEnv, jclass clz, jobject header, jstring data);
};


#endif//SOURCE_JAVAASSUTILS_H
