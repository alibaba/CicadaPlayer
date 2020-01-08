//
// Created by lifujun on 2020/1/7.
//

#include <utils/Android/FindClass.h>
#include <utils/Android/JniException.h>
#include "JavaLogger.h"
#include <utils/Android/JniEnv.h>
#include <cstring>
#include <utils/frame_work_log.h>

static char *nativePlayerPath = (char *) ("com/cicada/player/utils/Logger");

jclass gj_Logger_Class = nullptr;
jmethodID gj_Logger_OnLogCallback = nullptr;

void JavaLogger::init(JNIEnv *env)
{
    if (gj_Logger_Class == nullptr) {
        FindClass cls(env, nativePlayerPath);
        gj_Logger_Class = (jclass) env->NewGlobalRef(cls.getClass());
        gj_Logger_OnLogCallback = env->GetStaticMethodID(gj_Logger_Class,
                                  "nOnLogCallback",
                                  "(I[B)V");
    }

    log_set_back(JavaLogger::onLogCallback, nullptr);
}

void JavaLogger::unInit(JNIEnv *pEnv)
{
    if (gj_Logger_Class != nullptr) {
        pEnv->DeleteGlobalRef(gj_Logger_Class);
        gj_Logger_Class = nullptr;
        JniException::clearException(pEnv);
    }
}

static JNINativeMethod logger_method_table[] = {
    {"nSetLogLevel",      "(I)V", (void *) JavaLogger::java_SetLogLevel},
    {"nGetLogLevel",      "()I",  (void *) JavaLogger::java_GetLogLevel},
    {"nEnableConsoleLog", "(Z)V", (void *) JavaLogger::java_EnableConsoleLog},

};

int JavaLogger::registerMethod(JNIEnv *pEnv)
{
    if (gj_Logger_Class == nullptr) {
        return JNI_FALSE;
    }

    if (pEnv->RegisterNatives(gj_Logger_Class, logger_method_table,
                              sizeof(logger_method_table) / sizeof(JNINativeMethod)) < 0) {
        JniException::clearException(pEnv);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

void JavaLogger::java_SetLogLevel(JNIEnv *env, jclass jclazz, jint level)
{
    log_set_log_level((int) level);
}

int JavaLogger::java_GetLogLevel(JNIEnv *env, jclass jclazz)
{
    return log_get_level();
}

void JavaLogger::java_EnableConsoleLog(JNIEnv *env, jclass jclazz, jboolean enable)
{
    log_set_enable_console((bool) enable);
}


void JavaLogger::onLogCallback(void *userData, int prio, const char *buf)
{
    if (buf == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *pEnv = Jenv.getEnv();

    if (pEnv == nullptr) {
        return;
    }

    size_t len = strlen(buf);
    jbyteArray bufArray = pEnv->NewByteArray(len);
    pEnv->SetByteArrayRegion(bufArray, 0, len, (jbyte *) (buf));
    pEnv->CallStaticVoidMethod(gj_Logger_Class, gj_Logger_OnLogCallback, (jint) prio, bufArray);
    pEnv->DeleteLocalRef(bufArray);
    JniException::clearException(pEnv);
}

