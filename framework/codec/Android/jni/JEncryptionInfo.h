//
// Created by SuperMan on 2020/10/19.
//

#ifndef SOURCE_JENCRYPTIONINFO_H
#define SOURCE_JENCRYPTIONINFO_H


#include <jni.h>
#include <base/media/IAFPacket.h>

class JEncryptionInfo {
public:
    static void init(JNIEnv* env);

    static void unInit(JNIEnv* env);

public:
    static jobject convert(JNIEnv* env, IAFPacket::EncryptionInfo* info);
};


#endif //SOURCE_JENCRYPTIONINFO_H
