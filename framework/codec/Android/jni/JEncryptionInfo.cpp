//
// Created by SuperMan on 2020/10/19.
//

#include <utils/Android/FindClass.h>
#include <utils/Android/NewStringUTF.h>
#include <utils/Android/NewByteArray.h>
#include <utils/Android/NewLinkedList.h>
#include "JEncryptionInfo.h"

jclass jEncryptionInfo_class = nullptr;
jmethodID jEncryptionInfo_init = nullptr;
jfieldID jEncryptionInfo_crypt_byte_block = nullptr;
jfieldID jEncryptionInfo_skip_byte_block = nullptr;
jmethodID jEncryptionInfo_setScheme = nullptr;
jmethodID jEncryptionInfo_setKeyId = nullptr;
jmethodID jEncryptionInfo_setIv = nullptr;
jmethodID jEncryptionInfo_setSubsamples = nullptr;

jclass jSubsampleEncryptionInfo_class = nullptr;
jmethodID jSubsampleEncryptionInfo_init = nullptr;
jfieldID jSubsampleEncryptionInfo_bytes_of_clear_data = nullptr;
jfieldID jSubsampleEncryptionInfo_bytes_of_protected_data = nullptr;

void JEncryptionInfo::init(JNIEnv *env) {
    if (jEncryptionInfo_class == nullptr) {
        FindClass infoClass(env, "com/cicada/player/utils/media/EncryptionInfo");
        jEncryptionInfo_class = (jclass) (env->NewGlobalRef(infoClass.getClass()));

        jEncryptionInfo_init = env->GetMethodID(jEncryptionInfo_class, "<init>", "()V");
        jEncryptionInfo_setScheme = env->GetMethodID(jEncryptionInfo_class, "setScheme",
                                                     "(Ljava/lang/String;)V");
        jEncryptionInfo_crypt_byte_block = env->GetFieldID(jEncryptionInfo_class,
                                                           "crypt_byte_block", "I");
        jEncryptionInfo_skip_byte_block = env->GetFieldID(jEncryptionInfo_class, "skip_byte_block",
                                                          "I");
        jEncryptionInfo_setKeyId = env->GetMethodID(jEncryptionInfo_class, "setKeyId", "([B)V");
        jEncryptionInfo_setIv = env->GetMethodID(jEncryptionInfo_class, "setIv", "([B)V");
        jEncryptionInfo_setSubsamples = env->GetMethodID(jEncryptionInfo_class, "setSubsamples",
                                                         "(Ljava/lang/Object;)V");
    }
    if (jSubsampleEncryptionInfo_class == nullptr) {
        FindClass infoClass(env, "com/cicada/player/utils/media/SubsampleEncryptionInfo");
        jSubsampleEncryptionInfo_class = (jclass) (env->NewGlobalRef(infoClass.getClass()));
        jSubsampleEncryptionInfo_init = env->GetMethodID(jSubsampleEncryptionInfo_class, "<init>",
                                                         "()V");
        jSubsampleEncryptionInfo_bytes_of_clear_data = env->GetFieldID(
                jSubsampleEncryptionInfo_class, "bytes_of_clear_data", "I");
        jSubsampleEncryptionInfo_bytes_of_protected_data = env->GetFieldID(
                jSubsampleEncryptionInfo_class, "bytes_of_protected_data", "I");
    }


}

void JEncryptionInfo::unInit(JNIEnv *env) {
    if (jEncryptionInfo_class != nullptr) {
        env->DeleteGlobalRef(jEncryptionInfo_class);
        jEncryptionInfo_class = nullptr;
    }

    if (jSubsampleEncryptionInfo_class != nullptr) {
        env->DeleteGlobalRef(jSubsampleEncryptionInfo_class);
        jSubsampleEncryptionInfo_class = nullptr;
    }

}

jobject JEncryptionInfo::convert(JNIEnv *env,  IAFPacket::EncryptionInfo *info) {

    jobject jEncryptionInfo = env->NewObject(jEncryptionInfo_class, jEncryptionInfo_init);
    NewStringUTF scheme(env, info->scheme.c_str());
    env->CallVoidMethod(jEncryptionInfo, jEncryptionInfo_setScheme, scheme.getString());
    env->SetIntField(jEncryptionInfo, jEncryptionInfo_crypt_byte_block, info->crypt_byte_block);
    env->SetIntField(jEncryptionInfo, jEncryptionInfo_skip_byte_block, info->skip_byte_block);

    NewByteArray key(env, info->key_id, info->key_id_size);
    env->CallVoidMethod(jEncryptionInfo, jEncryptionInfo_setKeyId, key.getArray());

    NewByteArray iv(env, info->iv, info->iv_size);
    env->CallVoidMethod(jEncryptionInfo, jEncryptionInfo_setIv, iv.getArray());

    if (info->subsample_count > 0) {
        NewLinkedList subsmaplesList(env);
        for (auto &subsampeInfo : info->subsamples) {
            jobject jSubSampleInfo = env->NewObject(jSubsampleEncryptionInfo_class,
                                                    jSubsampleEncryptionInfo_init);
            env->SetIntField(jSubSampleInfo, jSubsampleEncryptionInfo_bytes_of_clear_data,
                             subsampeInfo.bytes_of_clear_data);
            env->SetIntField(jSubSampleInfo, jSubsampleEncryptionInfo_bytes_of_protected_data,
                             subsampeInfo.bytes_of_protected_data);

            subsmaplesList.add(jSubSampleInfo);
            env->DeleteLocalRef(jSubSampleInfo);
        }

        jobject pJobject = subsmaplesList.getList();
        env->CallVoidMethod(jEncryptionInfo , jEncryptionInfo_setSubsamples , pJobject);
    }

    return jEncryptionInfo;
}
