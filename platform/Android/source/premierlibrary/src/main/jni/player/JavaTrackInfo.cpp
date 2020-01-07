//
// Created by lifujun on 2018/12/7.
//

#include "JavaTrackInfo.h"
#include <utils/Android/NewStringUTF.h>
#include <utils/Android/FindClass.h>

static const char *trackInfoPath = "com/cicada/player/nativeclass/TrackInfo";

//STREAM_INFO
jclass gj_TrackInfoClass = nullptr;
jmethodID gj_TrackInfo_init = nullptr;
jmethodID gj_TrackInfo_setType = nullptr;

jfieldID gj_TrackInfo_Index = nullptr;
jfieldID gj_TrackInfo_Description = nullptr;

jfieldID gj_TrackInfo_VideoBitrate = nullptr;
jfieldID gj_TrackInfo_VideoWidth = nullptr;
jfieldID gj_TrackInfo_VideoHeight = nullptr;

jfieldID gj_TrackInfo_AudioLang = nullptr;
jfieldID gj_TrackInfo_AudioChannels = nullptr;
jfieldID gj_TrackInfo_AudioSampleRate = nullptr;
jfieldID gj_TrackInfo_AudioSampleFormat = nullptr;

jfieldID gj_TrackInfo_SubtitleLang = nullptr;


void JavaTrackInfo::init(JNIEnv *env)
{
    if (gj_TrackInfoClass == nullptr) {
        FindClass cls(env, trackInfoPath);
        gj_TrackInfoClass = (jclass) env->NewGlobalRef(cls.getClass());
        gj_TrackInfo_init = env->GetMethodID(gj_TrackInfoClass,
                                             "<init>",
                                             "()V");
        gj_TrackInfo_setType = env->GetMethodID(gj_TrackInfoClass,
                                                "setType",
                                                "(I)V");
        gj_TrackInfo_Index = env->GetFieldID(gj_TrackInfoClass, "index", "I");
        gj_TrackInfo_Description = env->GetFieldID(gj_TrackInfoClass, "description",
                                   "Ljava/lang/String;");
        gj_TrackInfo_VideoBitrate = env->GetFieldID(gj_TrackInfoClass, "videoBitrate", "I");
        gj_TrackInfo_VideoWidth = env->GetFieldID(gj_TrackInfoClass, "videoWidth", "I");
        gj_TrackInfo_VideoHeight = env->GetFieldID(gj_TrackInfoClass, "videoHeight", "I");
        gj_TrackInfo_AudioLang = env->GetFieldID(gj_TrackInfoClass, "audioLang",
                                 "Ljava/lang/String;");
        gj_TrackInfo_AudioChannels = env->GetFieldID(gj_TrackInfoClass, "audioChannels", "I");
        gj_TrackInfo_AudioSampleRate = env->GetFieldID(gj_TrackInfoClass, "audioSampleRate", "I");
        gj_TrackInfo_AudioSampleFormat = env->GetFieldID(gj_TrackInfoClass, "audioSampleFormat",
                                         "I");
        gj_TrackInfo_SubtitleLang = env->GetFieldID(gj_TrackInfoClass, "subtitleLang",
                                    "Ljava/lang/String;");
    }
}

void JavaTrackInfo::unInit(JNIEnv *pEnv)
{
    if (gj_TrackInfoClass != nullptr) {
        pEnv->DeleteGlobalRef(gj_TrackInfoClass);
        gj_TrackInfoClass = nullptr;
    }
}


jobject JavaTrackInfo::getTrackInfo(JNIEnv *mEnv, const StreamInfo &streamInfo)
{
    jobject jStreamInfo = mEnv->NewObject(gj_TrackInfoClass, gj_TrackInfo_init);
    mEnv->SetIntField(jStreamInfo, gj_TrackInfo_Index, streamInfo.streamIndex);
    mEnv->CallVoidMethod(jStreamInfo, gj_TrackInfo_setType, (int) streamInfo.type);
    NewStringUTF tmpdesc(mEnv, streamInfo.description);
    jstring desc = tmpdesc.getString();
    mEnv->SetObjectField(jStreamInfo, gj_TrackInfo_Description, desc);
    //video
    mEnv->SetIntField(jStreamInfo, gj_TrackInfo_VideoBitrate, streamInfo.videoBandwidth);
    mEnv->SetIntField(jStreamInfo, gj_TrackInfo_VideoHeight, streamInfo.videoHeight);
    mEnv->SetIntField(jStreamInfo, gj_TrackInfo_VideoWidth, streamInfo.videoWidth);
    //audio
    mEnv->SetIntField(jStreamInfo, gj_TrackInfo_AudioChannels, streamInfo.nChannels);
    mEnv->SetIntField(jStreamInfo, gj_TrackInfo_AudioSampleFormat, streamInfo.sampleFormat);
    mEnv->SetIntField(jStreamInfo, gj_TrackInfo_AudioSampleRate, streamInfo.sampleRate);
    NewStringUTF tmpaudioLang(mEnv, streamInfo.audioLang);
    jstring audioLang = tmpaudioLang.getString();
    mEnv->SetObjectField(jStreamInfo, gj_TrackInfo_AudioLang, audioLang);
    //subtitle
    NewStringUTF tmpsubtitleLang(mEnv, streamInfo.subtitleLang);
    jstring subtitleLang = tmpsubtitleLang.getString();
    mEnv->SetObjectField(jStreamInfo, gj_TrackInfo_SubtitleLang, subtitleLang);
    return jStreamInfo;
}

jobjectArray JavaTrackInfo::getTrackInfoArray(JNIEnv *mEnv, const std::vector<StreamInfo> &streamInfos,
        int len)
{
    jobjectArray jStreamInfoArray = mEnv->NewObjectArray(len, gj_TrackInfoClass, nullptr);

    for (int i = 0; i < len; i++) {
        jobject jStreamInfo = getTrackInfo(mEnv, streamInfos.at(i));

        if (jStreamInfo != nullptr) {
            mEnv->SetObjectArrayElement(jStreamInfoArray, i, jStreamInfo);
            mEnv->DeleteLocalRef(jStreamInfo);
        }
    }

    return jStreamInfoArray;
}

jobjectArray JavaTrackInfo::getTrackInfoArray(JNIEnv *mEnv, StreamInfo **streamInfos, int len)
{
    jobjectArray jStreamInfoArray = mEnv->NewObjectArray(len, gj_TrackInfoClass, nullptr);

    for (int i = 0; i < len; i++) {
        jobject jStreamInfo = getTrackInfo(mEnv, *(streamInfos[i]));

        if (jStreamInfo != nullptr) {
            mEnv->SetObjectArrayElement(jStreamInfoArray, i, jStreamInfo);
            mEnv->DeleteLocalRef(jStreamInfo);
        }
    }

    return jStreamInfoArray;
}

