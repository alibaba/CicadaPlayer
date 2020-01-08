//
// Created by lifujun on 2019/4/30.
//

#define LOG_TAG "JniException"

#include <utils/frame_work_log.h>
#include "JniException.h"


bool JniException::clearException(JNIEnv *env)
{
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
        return true;
    } else {
        return false;
    }
}
