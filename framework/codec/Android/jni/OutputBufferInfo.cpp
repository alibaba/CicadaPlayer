//
// Created by SuperMan on 2020/10/14.
//

#include <utils/Android/FindClass.h>
#include "OutputBufferInfo.h"

using namespace Cicada;


static jclass jOutputBufferInfoClass = nullptr;
static jfieldID jOutputBufferInfo_type = nullptr;
static jfieldID jOutputBufferInfo_index = nullptr;
static jfieldID jOutputBufferInfo_pts = nullptr;
static jfieldID jOutputBufferInfo_flags = nullptr;
static jfieldID jOutputBufferInfo_eos = nullptr;
static jfieldID jOutputBufferInfo_bufferSize = nullptr;
static jfieldID jOutputBufferInfo_bufferOffset = nullptr;
static jfieldID jOutputBufferInfo_videoWidth = nullptr;
static jfieldID jOutputBufferInfo_videoHeight = nullptr;
static jfieldID jOutputBufferInfo_videoStride = nullptr;
static jfieldID jOutputBufferInfo_videoSliceHeight = nullptr;
static jfieldID jOutputBufferInfo_videoPixelFormat = nullptr;
static jfieldID jOutputBufferInfo_videoCropLeft = nullptr;
static jfieldID jOutputBufferInfo_videoCropRight = nullptr;
static jfieldID jOutputBufferInfo_videoCropTop = nullptr;
static jfieldID jOutputBufferInfo_videoCropBottom = nullptr;
static jfieldID jOutputBufferInfo_audioChannelCount = nullptr;
static jfieldID jOutputBufferInfo_audioChannelMask = nullptr;
static jfieldID jOutputBufferInfo_audioSampleRate = nullptr;
static jfieldID jOutputBufferInfo_audioFormat = nullptr;


void OutputBufferInfo::init(JNIEnv *env) {
    if (env == nullptr) {
        return;
    }

    if (jOutputBufferInfoClass == nullptr) {
        FindClass jclass(env, "com/cicada/player/utils/media/OutputBufferInfo");
        jOutputBufferInfoClass = static_cast<_jclass *>(env->NewGlobalRef(jclass.getClass()));
        jOutputBufferInfo_type = env->GetFieldID(jOutputBufferInfoClass, "type", "I");
        jOutputBufferInfo_index = env->GetFieldID(jOutputBufferInfoClass, "index", "I");
        jOutputBufferInfo_pts = env->GetFieldID(jOutputBufferInfoClass, "pts", "J");
        jOutputBufferInfo_flags = env->GetFieldID(jOutputBufferInfoClass, "flags", "I");
        jOutputBufferInfo_eos = env->GetFieldID(jOutputBufferInfoClass, "eos", "Z");
        jOutputBufferInfo_bufferSize = env->GetFieldID(jOutputBufferInfoClass, "bufferSize", "I");
        jOutputBufferInfo_bufferOffset = env->GetFieldID(jOutputBufferInfoClass, "bufferOffset",
                                                         "I");
        jOutputBufferInfo_videoWidth = env->GetFieldID(jOutputBufferInfoClass, "videoWidth", "I");
        jOutputBufferInfo_videoHeight = env->GetFieldID(jOutputBufferInfoClass, "videoHeight", "I");
        jOutputBufferInfo_videoStride = env->GetFieldID(jOutputBufferInfoClass, "videoStride", "I");
        jOutputBufferInfo_videoSliceHeight = env->GetFieldID(jOutputBufferInfoClass,
                                                             "videoSliceHeight", "I");
        jOutputBufferInfo_videoPixelFormat = env->GetFieldID(jOutputBufferInfoClass,
                                                             "videoPixelFormat", "I");
        jOutputBufferInfo_videoCropLeft = env->GetFieldID(jOutputBufferInfoClass, "videoCropLeft",
                                                          "I");
        jOutputBufferInfo_videoCropRight = env->GetFieldID(jOutputBufferInfoClass, "videoCropRight",
                                                           "I");
        jOutputBufferInfo_videoCropTop = env->GetFieldID(jOutputBufferInfoClass, "videoCropTop",
                                                         "I");
        jOutputBufferInfo_videoCropBottom = env->GetFieldID(jOutputBufferInfoClass,
                                                            "videoCropBottom", "I");
        jOutputBufferInfo_audioChannelCount = env->GetFieldID(jOutputBufferInfoClass,
                                                              "audioChannelCount", "I");
        jOutputBufferInfo_audioChannelMask = env->GetFieldID(jOutputBufferInfoClass,
                                                             "audioChannelMask", "I");
        jOutputBufferInfo_audioSampleRate = env->GetFieldID(jOutputBufferInfoClass,
                                                            "audioSampleRate", "I");
        jOutputBufferInfo_audioFormat = env->GetFieldID(jOutputBufferInfoClass, "audioFormat", "I");
    }
}

void OutputBufferInfo::unInit(JNIEnv *env) {
    if (env == nullptr) {
        return;
    }

    if (jOutputBufferInfoClass != nullptr) {
        env->DeleteGlobalRef(jOutputBufferInfoClass);
        jOutputBufferInfoClass = nullptr;
    }
}

void OutputBufferInfo::convert(JNIEnv *env, OutputBufferInfo *dstInfo, jobject info) {
    if (env == nullptr || info == nullptr) {
        return;
    }

    dstInfo->type = env->GetIntField(info, jOutputBufferInfo_type);
    dstInfo->eos = env->GetBooleanField(info, jOutputBufferInfo_eos);
    dstInfo->index = env->GetIntField(info, jOutputBufferInfo_index);
    dstInfo->flags = env->GetIntField(info, jOutputBufferInfo_flags);
    dstInfo->pts = env->GetLongField(info, jOutputBufferInfo_pts);
    dstInfo->bufferSize = env->GetIntField(info, jOutputBufferInfo_bufferSize);
    dstInfo->bufferOffset = env->GetIntField(info, jOutputBufferInfo_bufferOffset);

    dstInfo->videoCropBottom = env->GetIntField(info, jOutputBufferInfo_videoCropBottom);
    dstInfo->videoCropTop = env->GetIntField(info, jOutputBufferInfo_videoCropTop);
    dstInfo->videoCropLeft = env->GetIntField(info, jOutputBufferInfo_videoCropLeft);
    dstInfo->videoCropRight = env->GetIntField(info, jOutputBufferInfo_videoCropRight);
    dstInfo->videoHeight = env->GetIntField(info, jOutputBufferInfo_videoHeight);
    dstInfo->videoWidth = env->GetIntField(info, jOutputBufferInfo_videoWidth);
    dstInfo->videoSliceHeight = env->GetIntField(info, jOutputBufferInfo_videoSliceHeight);
    dstInfo->videoStride = env->GetIntField(info, jOutputBufferInfo_videoStride);
    dstInfo->videoPixelFormat = env->GetIntField(info, jOutputBufferInfo_videoPixelFormat);

    dstInfo->audioChannelCount = env->GetIntField(info, jOutputBufferInfo_audioChannelCount);
    dstInfo->audioChannelMask = env->GetIntField(info, jOutputBufferInfo_audioChannelMask);
    dstInfo->audioSampleRate = env->GetIntField(info, jOutputBufferInfo_audioSampleRate);
    dstInfo->audioFormat = env->GetIntField(info, jOutputBufferInfo_audioFormat);
}
