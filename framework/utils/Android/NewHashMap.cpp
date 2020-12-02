//
// Created by SuperMan on 2020/10/23.
//

#include "NewHashMap.h"


#include "FindClass.h"

static const char *HashMapPath = "java/util/LinkedHashMap";

static jclass gj_HashMap_class = nullptr;
static jmethodID gj_HashMap_construct = nullptr;
static jmethodID gj_HashMap_put = nullptr;

NewHashMap::NewHashMap(JNIEnv *env) {
    if (env == nullptr) {
        return;
    }


    if (gj_HashMap_class == nullptr) {
        FindClass mapClass(env, HashMapPath);
        gj_HashMap_class = static_cast<jclass>(env->NewGlobalRef(mapClass.getClass()));
        gj_HashMap_construct = env->GetMethodID(gj_HashMap_class, "<init>", "()V");
        gj_HashMap_put = env->GetMethodID(gj_HashMap_class, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    }

    mEnv  = env;
    jobject object = env->NewObject(gj_HashMap_class, gj_HashMap_construct);
    mResult = env->NewGlobalRef(object);
    env->DeleteLocalRef(object);

}

NewHashMap::~NewHashMap() {
    if (mEnv != nullptr && mResult != nullptr) {
        mEnv->DeleteGlobalRef(mResult);
    }
}

jobject NewHashMap::getMap() {
    return mResult;
}

void NewHashMap::put(jobject key , jobject value ) {
    if (mEnv != nullptr && mResult != nullptr) {
        mEnv->CallObjectMethod(mResult, gj_HashMap_put, key, value);
    }
}