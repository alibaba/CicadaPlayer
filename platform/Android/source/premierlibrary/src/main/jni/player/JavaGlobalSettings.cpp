//
// Created by lifujun on 2020/4/9.
//

#include <utils/Android/FindClass.h>
#include <utils/globalSettings.h>
#include <utils/Android/GetStringUTFChars.h>
#include "JavaGlobalSettings.h"

static char *globalSettingsPath = (char *) ("com/cicada/player/CicadaPlayerGlobalSettings");
static jclass gj_GlobalSettings_Class = nullptr;

void JavaGlobalSettings::init(JNIEnv *pEnv)
{
    if (gj_GlobalSettings_Class == nullptr) {
        FindClass cls(pEnv, globalSettingsPath);
        gj_GlobalSettings_Class = (jclass) pEnv->NewGlobalRef(cls.getClass());
    }
}

void JavaGlobalSettings::unInit(JNIEnv *pEnv)
{
    if (gj_GlobalSettings_Class != nullptr) {
        pEnv->DeleteGlobalRef(gj_GlobalSettings_Class);
        gj_GlobalSettings_Class = nullptr;
    }
}

void JavaGlobalSettings::java_setDNSResolve(JNIEnv *mEnv, jclass clazz, jstring jhost, jstring jip)
{
    GetStringUTFChars hostStr(mEnv, jhost);
    char *hostChars = hostStr.getChars();

    if (hostChars == nullptr || strlen(hostChars) == 0) {
        return;
    }

    Cicada::globalSettings::getSetting()->removeResolve(hostChars, "");
    GetStringUTFChars ipStr(mEnv, jip);
    char *ipChars = ipStr.getChars();

    if (ipChars != nullptr && strlen(ipChars) > 0) {
        Cicada::globalSettings::getSetting()->addResolve(hostChars, ipChars);
    }
}


static JNINativeMethod globalSettings_method_table[] = {
    {"nSetDNSResolve",    "(Ljava/lang/String;Ljava/lang/String;)V", (void *) JavaGlobalSettings::java_setDNSResolve},
};

int JavaGlobalSettings::registerMethod(JNIEnv *pEnv)
{
    if (gj_GlobalSettings_Class == nullptr) {
        return JNI_FALSE;
    }

    if (pEnv->RegisterNatives(gj_GlobalSettings_Class, globalSettings_method_table,
                              sizeof(globalSettings_method_table) / sizeof(JNINativeMethod)) < 0) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}


