//
// Created by lifujun on 2018/12/10.
//

#include <utils/Android/FindClass.h>
#include "JavaMediaInfo.h"
#include "JavaTrackInfo.h"

static const char *MediaInfoPath = "com/cicada/player/nativeclass/MediaInfo";

//STREAM_INFO
jclass gj_MediaInfoClass = nullptr;
jmethodID gj_MediaInfo_init = nullptr;
jmethodID gj_MediaInfo_setTrackInfos = nullptr;
jmethodID gj_MediaInfo_getTrackInfoArray = nullptr;
jfieldID gj_MediaInfo_TotalBitRate = nullptr;

void JavaMediaInfo::init(JNIEnv *env) {
    if (gj_MediaInfoClass == nullptr) {
        FindClass cls(env, MediaInfoPath);
        gj_MediaInfoClass = (jclass) env->NewGlobalRef(cls.getClass());
        gj_MediaInfo_init = env->GetMethodID(gj_MediaInfoClass,
                                             "<init>",
                                             "()V");
        gj_MediaInfo_TotalBitRate = env->GetFieldID(gj_MediaInfoClass, "mTotalBitRate", "I");
        gj_MediaInfo_setTrackInfos = env->GetMethodID(gj_MediaInfoClass,
                                                      "setTrackInfos",
                                                      "([Lcom/cicada/player/nativeclass/TrackInfo;)V");
        gj_MediaInfo_getTrackInfoArray = env->GetMethodID(gj_MediaInfoClass,
                                                          "getTrackInfoArray",
                                                          "()[Ljava/lang/Object;");
    }
}

void JavaMediaInfo::unInit(JNIEnv *pEnv) {
    if (gj_MediaInfoClass != nullptr) {
        pEnv->DeleteGlobalRef(gj_MediaInfoClass);
        gj_MediaInfoClass = nullptr;
    }
}

jobject JavaMediaInfo::convertTo(JNIEnv *pEnv, const void *mediaInfo)
{
    jobject jmediaInfo = pEnv->NewObject(gj_MediaInfoClass, gj_MediaInfo_init);

    if (mediaInfo != nullptr) {
        auto *infos = (MediaInfo *) mediaInfo;
        jobjectArray trackInfos = JavaTrackInfo::getTrackInfoArray(pEnv, infos);
        pEnv->CallVoidMethod(jmediaInfo, gj_MediaInfo_setTrackInfos, trackInfos);
        pEnv->DeleteLocalRef(trackInfos);

        pEnv->SetIntField(jmediaInfo, gj_MediaInfo_TotalBitRate, infos->totalBitrate);
    }

    return jmediaInfo;
}

MediaInfo *JavaMediaInfo::convertToStream(JNIEnv *penv, jobject jmediaInfo)
{

    if (jmediaInfo == nullptr) {
        return nullptr;
    }

    auto tracks = static_cast<jobjectArray>(penv->CallObjectMethod(jmediaInfo, gj_MediaInfo_getTrackInfoArray));
    int length = penv->GetArrayLength(tracks);

    auto *pMediaInfo = new MediaInfo();
    pMediaInfo->totalBitrate = penv->GetIntField(jmediaInfo, gj_MediaInfo_TotalBitRate);

    for (int i = 0; i < length; i++) {
        jobject trackInfo = penv->GetObjectArrayElement(tracks, i);
        StreamInfo *info = JavaTrackInfo::getStreamInfo(penv, trackInfo);
        pMediaInfo->mStreamInfoQueue.push_back(info);
        penv->DeleteLocalRef(trackInfo);
    }

    penv->DeleteLocalRef(tracks);

    return pMediaInfo;
}
