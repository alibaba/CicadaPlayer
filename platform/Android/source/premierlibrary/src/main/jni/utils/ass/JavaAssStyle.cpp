//
// Created by SuperMan on 6/29/21.
//

#include "JavaAssStyle.h"
#include <utils/Android/FindClass.h>
#include <utils/Android/JniException.h>
#include <utils/Android/NewStringUTF.h>


static char *AssStylePath = (char *) ("com/cicada/player/utils/ass/AssStyle");

jclass gj_AssStyle_Class = nullptr;
jmethodID gj_AssStyle_init = nullptr;
jfieldID gj_AssStyle_Name = nullptr;
jfieldID gj_AssStyle_FontName = nullptr;
jfieldID gj_AssStyle_FontSize = nullptr;
jfieldID gj_AssStyle_PrimaryColour = nullptr;
jfieldID gj_AssStyle_SecondaryColour = nullptr;
jfieldID gj_AssStyle_OutlineColour = nullptr;
jfieldID gj_AssStyle_BackColour = nullptr;
jfieldID gj_AssStyle_Bold = nullptr;
jfieldID gj_AssStyle_Italic = nullptr;
jfieldID gj_AssStyle_Underline = nullptr;
jfieldID gj_AssStyle_StrikeOut = nullptr;
jfieldID gj_AssStyle_ScaleX = nullptr;
jfieldID gj_AssStyle_ScaleY = nullptr;
jfieldID gj_AssStyle_Spacing = nullptr;
jfieldID gj_AssStyle_Angle = nullptr;
jfieldID gj_AssStyle_BorderStyle = nullptr;
jfieldID gj_AssStyle_Outline = nullptr;
jfieldID gj_AssStyle_Shadow = nullptr;
jfieldID gj_AssStyle_Alignment = nullptr;
jfieldID gj_AssStyle_MarginL = nullptr;
jfieldID gj_AssStyle_MarginR = nullptr;
jfieldID gj_AssStyle_MarginV = nullptr;
jfieldID gj_AssStyle_Encoding = nullptr;

void JavaAssStyle::init(JNIEnv *env)
{
    if (gj_AssStyle_Class == nullptr) {
        FindClass cls(env, AssStylePath);
        gj_AssStyle_Class = (jclass) env->NewGlobalRef(cls.getClass());
        gj_AssStyle_init = env->GetMethodID(gj_AssStyle_Class, "<init>", "()V");
        gj_AssStyle_Name = env->GetFieldID(gj_AssStyle_Class, "mName", "Ljava/lang/String;");
        gj_AssStyle_FontName = env->GetFieldID(gj_AssStyle_Class, "mFontName", "Ljava/lang/String;");
        gj_AssStyle_FontSize = env->GetFieldID(gj_AssStyle_Class, "mFontSize", "D");
        gj_AssStyle_PrimaryColour = env->GetFieldID(gj_AssStyle_Class, "mPrimaryColour", "I");
        gj_AssStyle_SecondaryColour = env->GetFieldID(gj_AssStyle_Class, "mSecondaryColour", "I");
        gj_AssStyle_OutlineColour = env->GetFieldID(gj_AssStyle_Class, "mOutlineColour", "I");
        gj_AssStyle_BackColour = env->GetFieldID(gj_AssStyle_Class, "mBackColour", "I");
        gj_AssStyle_Bold = env->GetFieldID(gj_AssStyle_Class, "mBold", "I");
        gj_AssStyle_Italic = env->GetFieldID(gj_AssStyle_Class, "mItalic", "I");
        gj_AssStyle_Underline = env->GetFieldID(gj_AssStyle_Class, "mUnderline", "I");
        gj_AssStyle_StrikeOut = env->GetFieldID(gj_AssStyle_Class, "mStrikeOut", "I");
        gj_AssStyle_ScaleX = env->GetFieldID(gj_AssStyle_Class, "mScaleX", "D");
        gj_AssStyle_ScaleY = env->GetFieldID(gj_AssStyle_Class, "mScaleY", "D");
        gj_AssStyle_Spacing = env->GetFieldID(gj_AssStyle_Class, "mSpacing", "D");
        gj_AssStyle_Angle = env->GetFieldID(gj_AssStyle_Class, "mAngle", "D");
        gj_AssStyle_BorderStyle = env->GetFieldID(gj_AssStyle_Class, "mBorderStyle", "I");
        gj_AssStyle_Outline = env->GetFieldID(gj_AssStyle_Class, "mOutline", "D");
        gj_AssStyle_Shadow = env->GetFieldID(gj_AssStyle_Class, "mShadow", "D");
        gj_AssStyle_Alignment = env->GetFieldID(gj_AssStyle_Class, "mAlignment", "I");
        gj_AssStyle_MarginL = env->GetFieldID(gj_AssStyle_Class, "mMarginL", "I");
        gj_AssStyle_MarginR = env->GetFieldID(gj_AssStyle_Class, "mMarginR", "I");
        gj_AssStyle_MarginV = env->GetFieldID(gj_AssStyle_Class, "mMarginV", "I");
        gj_AssStyle_Encoding = env->GetFieldID(gj_AssStyle_Class, "mEncoding", "I");
    }
}

void JavaAssStyle::unInit(JNIEnv *pEnv)
{
    if (gj_AssStyle_Class != nullptr) {
        pEnv->DeleteGlobalRef(gj_AssStyle_Class);
        gj_AssStyle_Class = nullptr;
        JniException::clearException(pEnv);
    }
}

jobject JavaAssStyle::convertToJStyle(JNIEnv *pEnv, const Cicada::AssStyle &style)
{
    jobject jStyle = pEnv->NewObject(gj_AssStyle_Class, gj_AssStyle_init);
    NewStringUTF jName(pEnv, style.Name.c_str());
    pEnv->SetObjectField(jStyle, gj_AssStyle_Name, jName.getString());
    NewStringUTF jFontName(pEnv, style.FontName.c_str());
    pEnv->SetObjectField(jStyle, gj_AssStyle_FontName, jFontName.getString());

    pEnv->SetDoubleField(jStyle, gj_AssStyle_FontSize, style.FontSize);
    pEnv->SetIntField(jStyle, gj_AssStyle_PrimaryColour, style.PrimaryColour);
    pEnv->SetIntField(jStyle, gj_AssStyle_SecondaryColour, style.SecondaryColour);
    pEnv->SetIntField(jStyle, gj_AssStyle_OutlineColour, style.OutlineColour);
    pEnv->SetIntField(jStyle, gj_AssStyle_BackColour, style.BackColour);
    pEnv->SetIntField(jStyle, gj_AssStyle_Bold, style.Bold);
    pEnv->SetIntField(jStyle, gj_AssStyle_Italic, style.Italic);
    pEnv->SetIntField(jStyle, gj_AssStyle_Underline, style.Underline);
    pEnv->SetIntField(jStyle, gj_AssStyle_StrikeOut, style.StrikeOut);
    pEnv->SetDoubleField(jStyle, gj_AssStyle_ScaleX, style.ScaleX);
    pEnv->SetDoubleField(jStyle, gj_AssStyle_ScaleY, style.ScaleY);
    pEnv->SetDoubleField(jStyle, gj_AssStyle_Spacing, style.Spacing);
    pEnv->SetDoubleField(jStyle, gj_AssStyle_Angle, style.Angle);
    pEnv->SetIntField(jStyle, gj_AssStyle_BorderStyle, style.BorderStyle);
    pEnv->SetDoubleField(jStyle, gj_AssStyle_Outline, style.Outline);
    pEnv->SetDoubleField(jStyle, gj_AssStyle_Shadow, style.Shadow);
    pEnv->SetIntField(jStyle, gj_AssStyle_Alignment, style.Alignment);
    pEnv->SetIntField(jStyle, gj_AssStyle_MarginL, style.MarginL);
    pEnv->SetIntField(jStyle, gj_AssStyle_MarginR, style.MarginR);
    pEnv->SetIntField(jStyle, gj_AssStyle_MarginV, style.MarginV);
    pEnv->SetIntField(jStyle, gj_AssStyle_Encoding, style.Encoding);

    return jStyle;
}

void JavaAssStyle::convertToStyle(JNIEnv *pEnv, jobject pJobject, Cicada::AssStyle *pStyle)
{
    if (pJobject == nullptr || pStyle == nullptr) {
        return;
    }

    pStyle->FontSize = pEnv->GetDoubleField(pJobject, gj_AssStyle_FontSize);
    pStyle->PrimaryColour = pEnv->GetIntField(pJobject, gj_AssStyle_PrimaryColour);
    pStyle->SecondaryColour = pEnv->GetIntField(pJobject, gj_AssStyle_SecondaryColour);
    pStyle->OutlineColour = pEnv->GetIntField(pJobject, gj_AssStyle_OutlineColour);
    pStyle->BackColour = pEnv->GetIntField(pJobject, gj_AssStyle_BackColour);
    pStyle->Bold = pEnv->GetIntField(pJobject, gj_AssStyle_Bold);
    pStyle->Italic = pEnv->GetIntField(pJobject, gj_AssStyle_Italic);
    pStyle->Underline = pEnv->GetIntField(pJobject, gj_AssStyle_Underline);
    pStyle->StrikeOut = pEnv->GetIntField(pJobject, gj_AssStyle_StrikeOut);
    pStyle->ScaleX = pEnv->GetDoubleField(pJobject, gj_AssStyle_ScaleX);
    pStyle->ScaleY = pEnv->GetDoubleField(pJobject, gj_AssStyle_ScaleY);
    pStyle->Spacing = pEnv->GetDoubleField(pJobject, gj_AssStyle_Spacing);
    pStyle->Angle = pEnv->GetDoubleField(pJobject, gj_AssStyle_Angle);
    pStyle->BorderStyle = pEnv->GetIntField(pJobject, gj_AssStyle_BorderStyle);
    pStyle->Outline = pEnv->GetDoubleField(pJobject, gj_AssStyle_Outline);
    pStyle->Shadow = pEnv->GetDoubleField(pJobject, gj_AssStyle_Shadow);
    pStyle->Alignment = pEnv->GetIntField(pJobject, gj_AssStyle_Alignment);
    pStyle->MarginL = pEnv->GetIntField(pJobject, gj_AssStyle_MarginL);
    pStyle->MarginR = pEnv->GetIntField(pJobject, gj_AssStyle_MarginR);
    pStyle->MarginV = pEnv->GetIntField(pJobject, gj_AssStyle_MarginV);
    pStyle->Encoding = pEnv->GetIntField(pJobject, gj_AssStyle_Encoding);
}
