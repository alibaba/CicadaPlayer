//
// Created by SuperMan on 2020/10/23.
//

#ifndef SOURCE_NEWHASHMAP_H
#define SOURCE_NEWHASHMAP_H

#include <jni.h>
#include <utils/CicadaType.h>

class NewHashMap {

public:
    NewHashMap(JNIEnv *pEnv);

    ~NewHashMap();

public:
    jobject getMap();

    void put(jobject key , jobject value);

private:
    JNIEnv *mEnv{nullptr};
    jobject mResult{nullptr};

private:
    NewHashMap(NewHashMap &) {

    }

    const NewHashMap &operator=(const NewHashMap &);

};

#endif //SOURCE_NEWHASHMAP_H
