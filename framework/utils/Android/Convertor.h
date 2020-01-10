//
// Created by lifujun on 2019/3/29.
//

#ifndef SOURCE_CONVERTOR_H
#define SOURCE_CONVERTOR_H


#include <jni.h>
#include <string>
#include <map>

class Convertor {
public:
    static char* jByteArrayToChars(JNIEnv *env, jbyteArray bytearray);
    static char* jByteArrayToChars_New(JNIEnv *env, jbyteArray bytearray);

    static jobject cmap2Jmap(JNIEnv *env , std::map<std::string ,std::string> theMap);
};


#endif //SOURCE_CONVERTOR_H
