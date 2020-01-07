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

void JavaMediaInfo::init(JNIEnv *env)
{
    if (gj_MediaInfoClass == nullptr) {
        FindClass cls(env, MediaInfoPath);
        gj_MediaInfoClass = (jclass) env->NewGlobalRef(cls.getClass());
        gj_MediaInfo_init = env->GetMethodID(gj_MediaInfoClass,
                                             "<init>",
                                             "()V");
        gj_MediaInfo_setTrackInfos = env->GetMethodID(gj_MediaInfoClass,
                                     "setTrackInfos",
                                     "([Lcom/cicada/player/nativeclass/TrackInfo;)V");
    }
}

void JavaMediaInfo::unInit(JNIEnv *pEnv)
{
    if (gj_MediaInfoClass != nullptr) {
        pEnv->DeleteGlobalRef(gj_MediaInfoClass);
        gj_MediaInfoClass = nullptr;
    }
}

jobject JavaMediaInfo::convertTo(JNIEnv *pEnv, const void *streamInfos, int64_t count)
{
    jobject mediaInfo = pEnv->NewObject(gj_MediaInfoClass, gj_MediaInfo_init);

    if (count > 0 && streamInfos != nullptr) {
        auto **infos = (StreamInfo **) streamInfos;
        jobjectArray trackInfos = JavaTrackInfo::getTrackInfoArray(pEnv, infos, count);
        pEnv->CallVoidMethod(mediaInfo, gj_MediaInfo_setTrackInfos, trackInfos);
        pEnv->DeleteLocalRef(trackInfos);
    }

    return mediaInfo;
}
