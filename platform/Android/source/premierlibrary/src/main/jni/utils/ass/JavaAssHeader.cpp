//
// Created by SuperMan on 6/29/21.
//

#include "JavaAssHeader.h"
#include "JavaAssStyle.h"
#include <utils/Android/FindClass.h>
#include <utils/Android/GetStringUTFChars.h>
#include <utils/Android/JniException.h>
#include <utils/Android/NewHashMap.h>
#include <utils/Android/NewStringUTF.h>


static char *AssHeaderPath = (char *) ("com/cicada/player/utils/ass/AssHeader");

jclass gj_AssHeader_Class = nullptr;
jmethodID gj_AssHeader_init = nullptr;
jmethodID gj_AssHeader_setStyles = nullptr;
jmethodID gj_AssHeader_setType = nullptr;
jmethodID gj_AssHeader_getStyles = nullptr;
jmethodID gj_AssHeader_getType = nullptr;
jfieldID gj_AssHeader_PlayResX = nullptr;
jfieldID gj_AssHeader_PlayResY = nullptr;
jfieldID gj_AssHeader_Timer = nullptr;
jfieldID gj_AssHeader_WrapStyle = nullptr;
jfieldID gj_AssHeader_ScaledBorderAndShadow = nullptr;
jfieldID gj_AssHeader_StyleFormat = nullptr;
jfieldID gj_AssHeader_EventFormat = nullptr;

void JavaAssHeader::init(JNIEnv *env)
{
    if (gj_AssHeader_Class == nullptr) {
        FindClass cls(env, AssHeaderPath);
        gj_AssHeader_Class = (jclass) env->NewGlobalRef(cls.getClass());
        gj_AssHeader_init = env->GetMethodID(gj_AssHeader_Class, "<init>", "()V");
        gj_AssHeader_setStyles = env->GetMethodID(gj_AssHeader_Class, "setStyles", "(Ljava/lang/Object;)V");
        gj_AssHeader_setType = env->GetMethodID(gj_AssHeader_Class, "setType", "(I)V");
        gj_AssHeader_getStyles = env->GetMethodID(gj_AssHeader_Class, "getStyles", "()Ljava/lang/Object;");
        gj_AssHeader_getType = env->GetMethodID(gj_AssHeader_Class, "getType", "()I");

        gj_AssHeader_PlayResX = env->GetFieldID(gj_AssHeader_Class, "mPlayResX", "I");
        gj_AssHeader_PlayResY = env->GetFieldID(gj_AssHeader_Class, "mPlayResY", "I");
        gj_AssHeader_Timer = env->GetFieldID(gj_AssHeader_Class, "mTimer", "D");
        gj_AssHeader_WrapStyle = env->GetFieldID(gj_AssHeader_Class, "mWrapStyle", "I");
        gj_AssHeader_ScaledBorderAndShadow = env->GetFieldID(gj_AssHeader_Class, "mScaledBorderAndShadow", "I");
        gj_AssHeader_StyleFormat = env->GetFieldID(gj_AssHeader_Class, "mStyleFormat", "Ljava/lang/String;");
        gj_AssHeader_EventFormat = env->GetFieldID(gj_AssHeader_Class, "mEventFormat", "Ljava/lang/String;");
    }
}

void JavaAssHeader::unInit(JNIEnv *pEnv)
{
    if (gj_AssHeader_Class != nullptr) {
        pEnv->DeleteGlobalRef(gj_AssHeader_Class);
        gj_AssHeader_Class = nullptr;
        JniException::clearException(pEnv);
    }
}

jobject JavaAssHeader::convertToJHeader(JNIEnv *pEnv, const Cicada::AssHeader &header)
{
    jobject jHeader = pEnv->NewObject(gj_AssHeader_Class, gj_AssHeader_init);

    NewHashMap stylesMap(pEnv);
    for (const auto &iter : header.styles) {
        NewStringUTF key(pEnv, iter.first.c_str());
        jobject value = JavaAssStyle::convertToJStyle(pEnv, iter.second);
        stylesMap.put(key.getString(), value);
        pEnv->DeleteLocalRef(value);
    }

    pEnv->CallVoidMethod(jHeader, gj_AssHeader_setStyles, stylesMap.getMap());
    pEnv->CallVoidMethod(jHeader, gj_AssHeader_setType, header.Type);
    pEnv->SetIntField(jHeader, gj_AssHeader_PlayResX, header.PlayResX);
    pEnv->SetIntField(jHeader, gj_AssHeader_PlayResY, header.PlayResY);
    pEnv->SetDoubleField(jHeader, gj_AssHeader_Timer, header.Timer);
    pEnv->SetIntField(jHeader, gj_AssHeader_WrapStyle, header.WrapStyle);
    pEnv->SetIntField(jHeader, gj_AssHeader_ScaledBorderAndShadow, header.ScaledBorderAndShadow);
    NewStringUTF jStyleFormat(pEnv, header.styleFormat.c_str());
    pEnv->SetObjectField(jHeader, gj_AssHeader_StyleFormat, jStyleFormat.getString());
    NewStringUTF jEventFormat(pEnv, header.eventFormat.c_str());
    pEnv->SetObjectField(jHeader, gj_AssHeader_EventFormat, jEventFormat.getString());
    return jHeader;
}

void JavaAssHeader::covertToHeader(JNIEnv *pEnv, jobject pJobject, Cicada::AssHeader *pHeader)
{
    if (pJobject == nullptr || pHeader == nullptr) {
        return;
    }

    pHeader->PlayResX = (int) pEnv->GetIntField(pJobject, gj_AssHeader_PlayResX);
    pHeader->PlayResY = (int) pEnv->GetIntField(pJobject, gj_AssHeader_PlayResY);
    pHeader->Timer = (double) pEnv->GetDoubleField(pJobject, gj_AssHeader_Timer);
    pHeader->WrapStyle = (int) pEnv->GetIntField(pJobject, gj_AssHeader_WrapStyle);
    pHeader->ScaledBorderAndShadow = (int) pEnv->GetIntField(pJobject, gj_AssHeader_ScaledBorderAndShadow);
    pHeader->Type = static_cast<Cicada::SubtitleType>(pEnv->CallIntMethod(pJobject, gj_AssHeader_getType));
    {
        auto jStyleFormat = static_cast<jstring>(pEnv->GetObjectField(pJobject, gj_AssHeader_StyleFormat));
        GetStringUTFChars styleFormat(pEnv, jStyleFormat);
        pHeader->styleFormat = styleFormat.getChars() == nullptr ? "" : styleFormat.getChars();
    }

    {
        auto jEventFormat = static_cast<jstring>(pEnv->GetObjectField(pJobject, gj_AssHeader_EventFormat));
        GetStringUTFChars eventFormat(pEnv, jEventFormat);
        pHeader->eventFormat = eventFormat.getChars() == nullptr ? "" : eventFormat.getChars();
    }

    jobject styles = pEnv->CallObjectMethod(pJobject, gj_AssHeader_getStyles);
    pHeader->styles = covertStyles(pEnv, styles);
}

std::map<std::string, Cicada::AssStyle> JavaAssHeader::covertStyles(JNIEnv *env, jobject jobj)
{

    std::map<std::string, Cicada::AssStyle> cmap;

    if (jobj == nullptr) {
        return cmap;
    }

    FindClass jmapclass(env, "java/util/HashMap");
    jmethodID jkeysetmid = env->GetMethodID(jmapclass.getClass(), "keySet", "()Ljava/util/Set;");
    jmethodID jgetmid = env->GetMethodID(jmapclass.getClass(), "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
    jobject jsetkey = env->CallObjectMethod(jobj, jkeysetmid);
    FindClass jsetclass(env, "java/util/Set");
    jmethodID jtoArraymid = env->GetMethodID(jsetclass.getClass(), "toArray", "()[Ljava/lang/Object;");
    jobjectArray jobjArray = (jobjectArray) env->CallObjectMethod(jsetkey, jtoArraymid);
    if (jobjArray != nullptr) {
        jsize arraysize = env->GetArrayLength(jobjArray);
        int i = 0;
        for (i = 0; i < arraysize; i++) {
            jstring jkey = (jstring) env->GetObjectArrayElement(jobjArray, i);
            jobject jvalue = (jstring) env->CallObjectMethod(jobj, jgetmid, jkey);
            {
                GetStringUTFChars key(env, jkey);
                Cicada::AssStyle assStyle{};
                JavaAssStyle::convertToStyle(env, jvalue, &assStyle);
                cmap[key.getChars()] = assStyle;
            }
        }
    }
    if (jobjArray != nullptr) {
        env->DeleteLocalRef(jobjArray);
    }

    if (jsetkey != nullptr) {
        env->DeleteLocalRef(jsetkey);
    }

    return cmap;
}
