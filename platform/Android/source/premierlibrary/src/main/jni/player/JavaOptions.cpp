//
// Created by lifujun on 2020/7/23.
//

#include <utils/Android/FindClass.h>
#include <utils/Android/NewStringUTF.h>
#include "JavaOptions.h"

using namespace std;

static char *optionsPath = (char *) ("com/cicada/player/nativeclass/Options");

static jclass gj_Options_Class = nullptr;
static jmethodID gj_Options_init = nullptr;
static jmethodID gj_Options_set = nullptr;

void JavaOptions::init(JNIEnv *env) {
    if (gj_Options_Class == nullptr) {
        FindClass cls(env, optionsPath);
        gj_Options_Class = (jclass) env->NewGlobalRef(cls.getClass());
        gj_Options_init = env->GetMethodID(gj_Options_Class,
                                           "<init>",
                                           "()V");
        gj_Options_set = env->GetMethodID(gj_Options_Class,
                                          "set",
                                          "(Ljava/lang/String;Ljava/lang/String;I)Z");

    }
}

void JavaOptions::unInit(JNIEnv *pEnv) {
    if (gj_Options_Class != nullptr) {
        pEnv->DeleteGlobalRef(gj_Options_Class);
        gj_Options_Class = nullptr;
    }
}

jobject JavaOptions::convertTo(JNIEnv *pEnv, Cicada::options *options) {

    if (options == nullptr) {
        return nullptr;
    }

    jobject jOptions = pEnv->NewObject(gj_Options_Class, gj_Options_init);

    map<string, string> opts = options->getOptions();
    for (auto &opt : opts) {
        NewStringUTF key(pEnv, opt.first.c_str());
        NewStringUTF value(pEnv, opt.second.c_str());
        pEnv->CallBooleanMethod(jOptions, gj_Options_set, key.getString(), value.getString(), 1);
    }

    return jOptions;
}
