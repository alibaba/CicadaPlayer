//
// Created by SuperMan on 6/29/21.
//

#include "JavaAssUtils.h"
#include "JavaAssDialogue.h"
#include "JavaAssHeader.h"
#include <utils/Android/FindClass.h>
#include <utils/Android/GetStringUTFChars.h>
#include <utils/Android/JniException.h>


static char *AssUtilsPath = (char *) ("com/cicada/player/utils/ass/AssUtils");

jclass gj_AssUtils_Class = nullptr;

void JavaAssUtils::init(JNIEnv *env)
{
    if (gj_AssUtils_Class == nullptr) {
        FindClass cls(env, AssUtilsPath);
        gj_AssUtils_Class = (jclass) env->NewGlobalRef(cls.getClass());
    }
}

void JavaAssUtils::unInit(JNIEnv *pEnv)
{
    if (gj_AssUtils_Class != nullptr) {
        pEnv->DeleteGlobalRef(gj_AssUtils_Class);
        gj_AssUtils_Class = nullptr;
        JniException::clearException(pEnv);
    }
}

static JNINativeMethod assUtils_method_table[] = {
        {"nParseAssHeader", "(Ljava/lang/String;)Ljava/lang/Object;", (void *) JavaAssUtils::java_ParseAssHeader},
        {"nParseAssDialogue", "(Ljava/lang/Object;Ljava/lang/String;)Ljava/lang/Object;", (void *) JavaAssUtils::java_ParseAssDialogue},

};

int JavaAssUtils::registerMethod(JNIEnv *pEnv)
{
    if (gj_AssUtils_Class == nullptr) {
        return JNI_FALSE;
    }

    if (pEnv->RegisterNatives(gj_AssUtils_Class, assUtils_method_table, sizeof(assUtils_method_table) / sizeof(JNINativeMethod)) < 0) {
        JniException::clearException(pEnv);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

jobject JavaAssUtils::java_ParseAssHeader(JNIEnv *pEnv, jclass clz, jstring header)
{
    GetStringUTFChars headerStr(pEnv, header);
    Cicada::AssHeader assHeader = Cicada::AssUtils::parseAssHeader(headerStr.getChars() == nullptr ? "" : headerStr.getChars());
    jobject jHeader = JavaAssHeader::convertToJHeader(pEnv, assHeader);
    return jHeader;
}

jobject JavaAssUtils::java_ParseAssDialogue(JNIEnv *pEnv, jclass clz, jobject header, jstring data)
{
    Cicada::AssHeader headerObj{};
    JavaAssHeader::covertToHeader(pEnv, header, &headerObj);
    GetStringUTFChars dataStr(pEnv, data);
    Cicada::AssDialogue dialogue = Cicada::AssUtils::parseAssDialogue(headerObj, dataStr.getChars() == nullptr ? "" : dataStr.getChars());
    jobject jDialogue = JavaAssDialogue::convertToJDialogue(pEnv, dialogue);
    return jDialogue;
}
