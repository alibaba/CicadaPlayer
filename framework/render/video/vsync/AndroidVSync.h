//
// Created by lifujun on 2019/9/6.
//

#ifndef SOURCE_VYSNC_TIMER_16_H
#define SOURCE_VYSNC_TIMER_16_H

#include <render/video/vsync/IVSync.h>

#include <jni.h>

//api >= 16
class AndroidVSync : public IVSync {
public:

    static void init(JNIEnv* env);

    static void unInit(JNIEnv* env);

    explicit AndroidVSync(Listener &listener);

    ~AndroidVSync() override;

    void start() override;

    void pause() override;

    float getHz()override{
        return mHz;
    };

private:

    static int OnInit(JNIEnv *env, jclass jc, jlong ptr);

    static int OnVsync(JNIEnv *env, jclass jc, jlong ptr, jlong nanos);

    static void OnDestroy(JNIEnv *env, jclass jc, jlong ptr);

private:
    jobject mVsyncTimer = nullptr;
    static int64_t mPeriod;
    static float mHz;

};


#endif //SOURCE_VYSNC_TIMER_16_H
