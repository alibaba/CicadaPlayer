//
// Created by lifujun on 2018/12/10.
//

#ifndef SOURCE_JAVAMEDIAINFO_H
#define SOURCE_JAVAMEDIAINFO_H


#include <jni.h>
#include <utils/CicadaType.h>
#include <native_cicada_player_def.h>

class CICADA_CPLUS_EXTERN JavaMediaInfo {
public:

    static void init(JNIEnv *pEnv);

    static void unInit(JNIEnv *pEnv);

    static jobject convertTo(JNIEnv *pEnv, const void *mediaInfo);

    static MediaInfo *convertToStream(JNIEnv *penv, jobject jmediaInfo);
};


#endif //SOURCE_JAVAMEDIAINFO_H
