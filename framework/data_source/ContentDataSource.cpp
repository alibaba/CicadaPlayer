//
// Created by SuperMan on 2020/9/21.
//

#include <utils/Android/FindClass.h>
#include <utils/Android/JniEnv.h>
#include <utils/Android/NewStringUTF.h>
#include <utils/Android/JniException.h>
#include "ContentDataSource.h"

using namespace Cicada;

ContentDataSource ContentDataSource::se(0);

jclass jContentDataSourceClass = nullptr;
jmethodID jContentDataSource_init = nullptr;
jmethodID jContentDataSource_setUri = nullptr;
jmethodID jContentDataSource_open = nullptr;
jmethodID jContentDataSource_close = nullptr;
jmethodID jContentDataSource_read = nullptr;
jmethodID jContentDataSource_seek = nullptr;

ContentDataSource::ContentDataSource(const std::string &url) : IDataSource(url) {
}

ContentDataSource::~ContentDataSource() {

}

int ContentDataSource::Open(int flags) {
    JniEnv env{};
    JNIEnv *pEnv = env.getEnv();
    if (pEnv == nullptr) {
        return -EINVAL;
    }

    if (jContentDataSourceClass == nullptr) {
        return -EINVAL;
    }

    jobject jContent = pEnv->NewObject(jContentDataSourceClass, jContentDataSource_init);
    mJContentDataSource = pEnv->NewGlobalRef(jContent);
    pEnv->DeleteLocalRef(jContent);

    NewStringUTF jurl(pEnv, mUri.c_str());
    pEnv->CallVoidMethod(mJContentDataSource, jContentDataSource_setUri, jurl.getString());

    jint ret = pEnv->CallIntMethod(mJContentDataSource, jContentDataSource_open, (jint) flags);
    JniException::clearException(pEnv);
    return (int) ret;
}

void ContentDataSource::Close() {
    JniEnv env{};
    JNIEnv *pEnv = env.getEnv();
    if (pEnv == nullptr) {
        return;
    }

    pEnv->CallVoidMethod(mJContentDataSource, jContentDataSource_close);
    JniException::clearException(pEnv);
}

int64_t ContentDataSource::Seek(int64_t offset, int whence) {
    JniEnv env{};
    JNIEnv *pEnv = env.getEnv();
    if (pEnv == nullptr) {
        return -EINVAL;
    }

    jlong ret = pEnv->CallLongMethod(mJContentDataSource, jContentDataSource_seek, (jlong) offset,
                                     (jint) whence);
    JniException::clearException(pEnv);
    return (int64_t) ret;
}

int ContentDataSource::Read(void *buf, size_t nbyte) {
    JniEnv env{};
    JNIEnv *pEnv = env.getEnv();
    if (pEnv == nullptr) {
        return -EINVAL;
    }

    jbyteArray buffer = pEnv->NewByteArray((jsize) nbyte);
    int readSize = (int) pEnv->CallIntMethod(mJContentDataSource, jContentDataSource_read, buffer);
    if (readSize <= 0) {
        return readSize;
    }
    jboolean isCopy = false;
    jbyte *jBytes = pEnv->GetByteArrayElements(buffer, &isCopy);
    memcpy(buf, jBytes, readSize);

    pEnv->ReleaseByteArrayElements(buffer, jBytes, 0);
    pEnv->DeleteLocalRef(buffer);
    return readSize;
}
void ContentDataSource::Interrupt(bool interrupt) {
    IDataSource::Interrupt(interrupt);
}

std::string ContentDataSource::Get_error_info(int error) {
    return IDataSource::Get_error_info(error);
}

std::string ContentDataSource::GetOption(const std::string &key) {
    return IDataSource::GetOption(key);
}

void ContentDataSource::init() {

    if (jContentDataSourceClass != nullptr) {
        return;
    }

    JniEnv env{};
    JNIEnv *pEnv = env.getEnv();
    if (pEnv == nullptr) {
        return;
    }

    FindClass nativePlayerClass(pEnv, "com/cicada/player/utils/ContentDataSource");
    jclass dataSourceClass = nativePlayerClass.getClass();
    if (dataSourceClass == nullptr) {
        return;
    }
    jContentDataSourceClass = static_cast<jclass>(pEnv->NewGlobalRef(dataSourceClass));
    jContentDataSource_init = pEnv->GetMethodID(jContentDataSourceClass, "<init>", "()V");
    jContentDataSource_setUri = pEnv->GetMethodID(jContentDataSourceClass, "setUri",
                                                  "(Ljava/lang/String;)V");
    jContentDataSource_open = pEnv->GetMethodID(jContentDataSourceClass, "open", "(I)I");
    jContentDataSource_read = pEnv->GetMethodID(jContentDataSourceClass, "read", "([B)I");
    jContentDataSource_seek = pEnv->GetMethodID(jContentDataSourceClass, "seek", "(JI)J");
    jContentDataSource_close = pEnv->GetMethodID(jContentDataSourceClass, "close", "()V");
}


void ContentDataSource::unInit() {
    JniEnv env{};
    JNIEnv *pEnv = env.getEnv();
    if (pEnv == nullptr) {
        return;
    }
    if (jContentDataSourceClass != nullptr) {
        pEnv->DeleteGlobalRef(jContentDataSourceClass);
        jContentDataSourceClass = nullptr;
    }
}
