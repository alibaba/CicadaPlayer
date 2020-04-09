//
// Created by lifujun on 2020/4/9.
//

#ifndef SOURCE_JAVAGLOBALSETTINGS_H
#define SOURCE_JAVAGLOBALSETTINGS_H


#include <jni.h>

class JavaGlobalSettings {

public:
    static void init(JNIEnv *pEnv);

    static void unInit(JNIEnv *pEnv);

    static int registerMethod(JNIEnv *pEnv);

    static void java_setDNSResolve(JNIEnv *mEnv, jclass clazz, jstring host, jstring ip);
};


#endif //SOURCE_JAVAGLOBALSETTINGS_H
