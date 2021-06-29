//
// Created by SuperMan on 6/29/21.
//

#ifndef SOURCE_JAVAASSSTYLE_H
#define SOURCE_JAVAASSSTYLE_H


#include <jni.h>
#include <utils/AssUtils.h>

class JavaAssStyle {
public:
    static void init(JNIEnv *pEnv);

    static void unInit(JNIEnv *pEnv);

    static jobject convertToJStyle(JNIEnv *pEnv, const Cicada::AssStyle &style);

    static void convertToStyle(JNIEnv *pEnv, jobject pJobject, Cicada::AssStyle *pStyle);
};


#endif//SOURCE_JAVAASSSTYLE_H
