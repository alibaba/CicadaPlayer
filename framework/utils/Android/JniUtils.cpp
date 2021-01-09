//
// Created by lifujun on 2019/3/29.
//

#include <cstring>
#include <malloc.h>
#include "JniUtils.h"
#include "JniException.h"
#include "FindClass.h"
#include "NewStringUTF.h"
#include "CallObjectMethod.h"
#include "GetStringUTFChars.h"

char *JniUtils::jByteArrayToChars(JNIEnv *env, jbyteArray bytearray) {
    jbyte *bytes = env->GetByteArrayElements(bytearray, 0);
    int chars_len = env->GetArrayLength(bytearray);
    char *chars = static_cast<char *>(malloc(chars_len + 1));
    memcpy(chars, bytes, chars_len);
    chars[chars_len] = 0;
    env->ReleaseByteArrayElements(bytearray, bytes, 0);
    JniException::clearException(env);
    return chars;
}

char *JniUtils::jByteArrayToChars_New(JNIEnv *env, jbyteArray bytearray) {
    jbyte *bytes = env->GetByteArrayElements(bytearray, 0);
    int chars_len = env->GetArrayLength(bytearray);
    char *chars = new char[chars_len + 1]();
    memcpy(chars, bytes, chars_len);
    chars[chars_len] = 0;
    env->ReleaseByteArrayElements(bytearray, bytes, 0);
    JniException::clearException(env);
    return chars;
}

jobject JniUtils::cmap2Jmap(JNIEnv *env, std::map<std::string, std::string> cmap) {
    FindClass jmapclass(env, "java/util/HashMap");
    jmethodID initMethod = env->GetMethodID(jmapclass.getClass(), "<init>", "()V");
    jmethodID putMethod = env->GetMethodID(jmapclass.getClass(), "put",
                                           "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    jobject jmap = env->NewObject(jmapclass.getClass(), initMethod, "");
    std::map<std::string, std::string>::iterator it;

    for (it = cmap.begin(); it != cmap.end(); it++) {
        std::string key = it->first;
        std::string value = it->second;
        NewStringUTF jKey(env, key.c_str());
        NewStringUTF jValue(env, value.c_str());
        env->CallObjectMethod(jmap, putMethod, jKey.getString(), jValue.getString());
    }

    return jmap;
}


std::string JniUtils::callStringMethod(JNIEnv *env, jobject jObj, jmethodID method) {
    CallObjectMethod tmpGetObject(env, jObj, method);
    auto objec = (jstring) tmpGetObject.getValue();
    GetStringUTFChars tmpObj(env, objec);
    char *ch_Obj = tmpObj.getChars();
    return ch_Obj == nullptr ? "" : std::string(ch_Obj);
}