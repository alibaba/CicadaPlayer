//
// Created by SuperMan on 6/29/21.
//

#include "JavaAssDialogue.h"
#include <utils/Android/FindClass.h>
#include <utils/Android/JniException.h>
#include <utils/Android/NewStringUTF.h>

static char *AssDialoguePath = (char *) ("com/cicada/player/utils/ass/AssDialogue");

jclass gj_AssDialogue_Class = nullptr;
jmethodID gj_AssDialogue_init = nullptr;
jfieldID gj_AssDialogue_Layer = nullptr;
jfieldID gj_AssDialogue_Style = nullptr;
jfieldID gj_AssDialogue_Name = nullptr;
jfieldID gj_AssDialogue_MarginL = nullptr;
jfieldID gj_AssDialogue_MarginR = nullptr;
jfieldID gj_AssDialogue_MarginV = nullptr;
jfieldID gj_AssDialogue_Effect = nullptr;
jfieldID gj_AssDialogue_Text = nullptr;

void JavaAssDialogue::init(JNIEnv *env)
{
    if (gj_AssDialogue_Class == nullptr) {
        FindClass cls(env, AssDialoguePath);
        gj_AssDialogue_Class = (jclass) env->NewGlobalRef(cls.getClass());
        gj_AssDialogue_init = env->GetMethodID(gj_AssDialogue_Class, "<init>", "()V");

        gj_AssDialogue_Layer = env->GetFieldID(gj_AssDialogue_Class, "mLayer", "I");
        gj_AssDialogue_Style = env->GetFieldID(gj_AssDialogue_Class, "mStyle", "Ljava/lang/String;");
        gj_AssDialogue_Name = env->GetFieldID(gj_AssDialogue_Class, "mName", "Ljava/lang/String;");
        gj_AssDialogue_MarginL = env->GetFieldID(gj_AssDialogue_Class, "mMarginL", "I");
        gj_AssDialogue_MarginR = env->GetFieldID(gj_AssDialogue_Class, "mMarginR", "I");
        gj_AssDialogue_MarginV = env->GetFieldID(gj_AssDialogue_Class, "mMarginV", "I");
        gj_AssDialogue_Effect = env->GetFieldID(gj_AssDialogue_Class, "mEffect", "Ljava/lang/String;");
        gj_AssDialogue_Text = env->GetFieldID(gj_AssDialogue_Class, "mText", "Ljava/lang/String;");
    }
}

void JavaAssDialogue::unInit(JNIEnv *pEnv)
{
    if (gj_AssDialogue_Class != nullptr) {
        pEnv->DeleteGlobalRef(gj_AssDialogue_Class);
        gj_AssDialogue_Class = nullptr;
        JniException::clearException(pEnv);
    }
}

jobject JavaAssDialogue::convertToJDialogue(JNIEnv *pEnv, const Cicada::AssDialogue &dialogue)
{
    jobject jDialogue = pEnv->NewObject(gj_AssDialogue_Class, gj_AssDialogue_init);

    pEnv->SetIntField(jDialogue, gj_AssDialogue_Layer, dialogue.Layer);
    NewStringUTF jStyle(pEnv, dialogue.Style.c_str());
    pEnv->SetObjectField(jDialogue, gj_AssDialogue_Style, jStyle.getString());
    NewStringUTF jName(pEnv, dialogue.Name.c_str());
    pEnv->SetObjectField(jDialogue, gj_AssDialogue_Name, jName.getString());
    pEnv->SetIntField(jDialogue, gj_AssDialogue_MarginL, dialogue.MarginL);
    pEnv->SetIntField(jDialogue, gj_AssDialogue_MarginR, dialogue.MarginR);
    pEnv->SetIntField(jDialogue, gj_AssDialogue_MarginV, dialogue.MarginV);
    NewStringUTF jEffect(pEnv, dialogue.Effect.c_str());
    pEnv->SetObjectField(jDialogue, gj_AssDialogue_Effect, jEffect.getString());
    NewStringUTF jText(pEnv, dialogue.Text.c_str());
    pEnv->SetObjectField(jDialogue, gj_AssDialogue_Text, jText.getString());

    return jDialogue;
}
