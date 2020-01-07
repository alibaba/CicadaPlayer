//
// Created by lifujun on 2019/4/30.
//

#ifndef SOURCE_JNIEXCEPTION_H
#define SOURCE_JNIEXCEPTION_H

#include <jni.h>
#include <utils/CicadaType.h>

class CICADA_CPLUS_EXTERN JniException {
public:
    static bool clearException(JNIEnv * env);
};


#endif //SOURCE_JNIEXCEPTION_H
