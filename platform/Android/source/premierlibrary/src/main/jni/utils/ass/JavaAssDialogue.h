//
// Created by SuperMan on 6/29/21.
//

#ifndef SOURCE_JAVAASSDIALOGUE_H
#define SOURCE_JAVAASSDIALOGUE_H


#include <jni.h>
#include <utils/AssUtils.h>

class JavaAssDialogue {
public:
    static void init(JNIEnv *pEnv);

    static void unInit(JNIEnv *pEnv);

    static jobject convertToJDialogue(JNIEnv *pEnv, const Cicada::AssDialogue &dialogue);
};


#endif//SOURCE_JAVAASSDIALOGUE_H
