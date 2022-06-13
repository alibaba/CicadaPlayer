//
// Created by SuperMan on 6/29/21.
//

#ifndef SOURCE_JAVAASSHEADER_H
#define SOURCE_JAVAASSHEADER_H


#include <jni.h>
#include <utils/AssUtils.h>

class JavaAssHeader {
public:
    static void init(JNIEnv *pEnv);

    static void unInit(JNIEnv *pEnv);

    static jobject convertToJHeader(JNIEnv *pEnv, const Cicada::AssHeader &header);

    static void covertToHeader(JNIEnv *pEnv, jobject pJobject, Cicada::AssHeader *pHeader);

    static std::map<std::string, Cicada::AssStyle> covertStyles(JNIEnv *pEnv, jobject pJobject);
};


#endif//SOURCE_JAVAASSHEADER_H
