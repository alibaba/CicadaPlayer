//
// Created by SuperMan on 2020/10/19.
//

#include "NewLinkedList.h"

#include "NewLinkedList.h"
#include "FindClass.h"

static const char *LinkedListPath = "java/util/LinkedList";

static jclass gj_linkedList_class = nullptr;
static jmethodID gj_linkedList_construct = nullptr;
static jmethodID gj_linkedList_add = nullptr;

NewLinkedList::NewLinkedList(JNIEnv *env) {
    if (env == nullptr) {
        return;
    }

    if (gj_linkedList_class == nullptr) {
        FindClass listClass(env, LinkedListPath);
        gj_linkedList_class = static_cast<jclass>(env->NewGlobalRef(listClass.getClass()));
        gj_linkedList_construct = env->GetMethodID(gj_linkedList_class, "<init>", "()V");
        gj_linkedList_add = env->GetMethodID(gj_linkedList_class, "add", "(Ljava/lang/Object;)Z");
    }

    mEnv  = env;
    jobject object = env->NewObject(gj_linkedList_class, gj_linkedList_construct, "");
    mResult = env->NewGlobalRef(object);
    env->DeleteLocalRef(object);

}

NewLinkedList::~NewLinkedList() {
    if (mEnv != nullptr && mResult != nullptr) {
        mEnv->DeleteGlobalRef(mResult);
    }
}

jobject NewLinkedList::getList() {
    return mResult;
}

void NewLinkedList::add(jobject value) {
    if (mEnv != nullptr && mResult != nullptr) {
        mEnv->CallBooleanMethod(mResult, gj_linkedList_add, value);
    }
}