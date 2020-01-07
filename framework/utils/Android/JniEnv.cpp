//
// Created by lifujun on 2019/9/24.
//

#include <cstdlib>
#include <pthread.h>
#include <sys/prctl.h>
#include <cstring>
#include "JniEnv.h"


#define MAX_TASK_NAME_LEN (16)

static int attachCount = 0;

static pthread_key_t g_threadKey;
JavaVM *g_vm = nullptr;

JniEnv::JniEnv() : mEnv(nullptr) {
    int status;

    if (g_vm == nullptr)
        return;

    status = (g_vm)->GetEnv((void **) &mEnv, JNI_VERSION_1_4);

    if (status >= 0)
        return;

    JavaVMAttachArgs lJavaVMAttachArgs;
    lJavaVMAttachArgs.version = JNI_VERSION_1_4;
    int size = MAX_TASK_NAME_LEN + 1;
    char *name = static_cast<char *>(malloc(size));
    memset(name, 0, size);

    if (prctl(PR_GET_NAME, (unsigned long) name, 0, 0, 0) != 0) {
        strcpy(name, "<name unknown>");
    } else {
        name[MAX_TASK_NAME_LEN] = 0;
    }

//    AF_LOGI("attach Thread name = %s \n", name);
    lJavaVMAttachArgs.name = name;
    lJavaVMAttachArgs.group = nullptr;
    status = (g_vm)->AttachCurrentThread(&mEnv, &lJavaVMAttachArgs);
    free(name);

    if (status < 0) {
//        AF_LOGI("failed to attach current thread \n");
        return;
    }

    attachCount++;
//    AF_LOGE("AttachCurrentThread tid=%d , attachCount = %d \n", gettid(), attachCount);
    pthread_setspecific(g_threadKey, mEnv);
}

JniEnv::~JniEnv() {
    if (g_vm == nullptr)
        return;
}

JNIEnv *JniEnv::getEnv() {
    return mEnv;
}

void JniEnv::JNI_ThreadDestroyed(void *value) {
    /* The thread is being destroyed, detach it from the Java VM and set the ThreadKey value to nullptr as required */
    JNIEnv *env = (JNIEnv *) value;

    if (env != nullptr && g_vm != nullptr) {
        attachCount--;
        g_vm->DetachCurrentThread();
        pthread_setspecific(g_threadKey, nullptr);
    }
}

void JniEnv::init(JavaVM *vm) {
    g_vm = vm;
    pthread_key_create(&g_threadKey, JNI_ThreadDestroyed);
}
