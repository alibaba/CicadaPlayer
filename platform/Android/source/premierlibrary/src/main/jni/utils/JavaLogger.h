//
// Created by lifujun on 2020/1/7.
//

#ifndef SOURCE_JAVALOGGER_H
#define SOURCE_JAVALOGGER_H


#include <jni.h>

class JavaLogger {
public:

    static void init(JNIEnv *pEnv);

    static void unInit(JNIEnv *pEnv);

    static int registerMethod(JNIEnv *pEnv);


    static void java_SetLogLevel(JNIEnv *env, jclass jclazz, jint level);

    static int java_GetLogLevel(JNIEnv *env, jclass jclazz);

    static void java_EnableConsoleLog(JNIEnv *env, jclass jclazzm, jboolean enable);

    static void onLogCallback(void *userData, int prio, const char *buf);
};


#endif //SOURCE_JAVALOGGER_H
