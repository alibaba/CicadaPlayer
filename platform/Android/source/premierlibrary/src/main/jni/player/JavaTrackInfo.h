//
// Created by lifujun on 2018/12/7.
//

#ifndef SOURCE_JAVATRACKINFO_H
#define SOURCE_JAVATRACKINFO_H


#include <jni.h>
#include <cstddef>
#include <utils/CicadaType.h>
#include <native_cicada_player_def.h>

class CICADA_CPLUS_EXTERN JavaTrackInfo {

public:
    static void init(JNIEnv *pEnv);

    static void unInit(JNIEnv *pEnv);

    static jobject getTrackInfo(JNIEnv *mEnv, const StreamInfo &streamInfo);

    static StreamInfo* getStreamInfo(JNIEnv *mEnv,jobject trackInfo);

    static int getStreamIndexByTrackInfo(JNIEnv *mEnv,jobject trackInfo);

    static jobjectArray
    getTrackInfoArray(JNIEnv *mEnv, const std::vector<StreamInfo> &streamInfos, int len);

    static jobjectArray getTrackInfoArray(JNIEnv *mEnv, StreamInfo **streamInfos, int len);
};


#endif //SOURCE_JAVATRACKINFO_H
