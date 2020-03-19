//
// Created by lifujun on 2019/1/7.
//

#ifndef SOURCE_NAITVEBASE_H
#define SOURCE_NAITVEBASE_H

#include <jni.h>
#include <android/native_window.h>

#include <MediaPlayer.h>

static char *nativePlayerPath = (char *) ("com/cicada/player/nativeclass/NativePlayerBase");

struct PlayerPrivateData {
    Cicada::MediaPlayer *player = nullptr;
    jobject j_instance = nullptr;
    ANativeWindow *nativeWindow = nullptr;
    jobject mSurfaceRef = nullptr;
};


/**
 * functions used by JniSaasPlayer and JniSaasListPlayer
 */
class NativeBase {


public:

    static Cicada::MediaPlayer *getPlayer(JNIEnv *env, jobject instance);

    static void init(JNIEnv *pEnv);

    static void unInit(JNIEnv *pEnv);

    static int registerMethod(JNIEnv *pEnv);


public:
    static void java_Construct(JNIEnv *env, jobject instance);

    static void java_SetConnectivityManager(JNIEnv *env, jobject instance, jobject connectManager);

    static void java_EnableHardwareDecoder(JNIEnv *env, jobject instance,
                                           jboolean enabled);

    static void java_SetView(JNIEnv *env, jobject instance, jobject surface);

    static void java_SetDataSource(JNIEnv *env, jobject instance, jstring urlSource);

    static void java_AddExtSubtitle(JNIEnv *env, jobject instance, jstring url);

    static void java_SelectExtSubtitle(JNIEnv *env, jobject instance, jint index, jboolean select);

    static void java_SelectTrack(JNIEnv *env, jobject instance, jint index);

    static void java_Prepare(JNIEnv *env, jobject instance);

    static void java_Start(JNIEnv *env, jobject instance);

    static void java_Pause(JNIEnv *env, jobject instance);

    static void java_SetVolume(JNIEnv *env, jobject instance, jfloat volume);

    static jfloat java_GetVolume(JNIEnv *env, jobject instance);

    static void java_SeekTo(JNIEnv *env, jobject instance, jlong position, jint mode);

    static void java_SetMaxAccurateSeekDelta(JNIEnv *env, jobject instance, jint delta);

    static void java_Stop(JNIEnv *env, jobject instance);

    static void java_Release(JNIEnv *env, jobject instance);

    static jlong java_GetDuration(JNIEnv *env, jobject instance);

    static jobject java_GetCurrentStreamInfo(JNIEnv *env, jobject instance, int streamType);

    static jlong java_GetCurrentPosition(JNIEnv *env, jobject instance);

    static jlong java_GetBufferedPosition(JNIEnv *env, jobject instance);

    static void java_SetMute(JNIEnv *env, jobject instance, jboolean mute);

    static jboolean java_IsMuted(JNIEnv *env, jobject instance);

    static void java_SetConfig(JNIEnv *env, jobject instance, jobject playerConfig);

    static jobject java_GetConfig(JNIEnv *env, jobject instance);

    static void java_SetCacheConfig(JNIEnv *env, jobject instance, jobject cacheConfig);

    static void java_SetScaleMode(JNIEnv *env, jobject instance, jint mode);

    static jint java_GetScaleMode(JNIEnv *env, jobject instance);

    static void java_SetLoop(JNIEnv *env, jobject instance, jboolean on);

    static jboolean java_IsLoop(JNIEnv *env, jobject instance);

    static jint java_GetVideoWidth(JNIEnv *env, jobject instance);

    static jint java_GetVideoHeight(JNIEnv *env, jobject instance);

    static jint java_GetVideoRotation(JNIEnv *env, jobject instance);

    static void java_Reload(JNIEnv *env, jobject instance);

    static void java_SetRotateMode(JNIEnv *env, jobject instance, jint mode);

    static jint java_GetRotateMode(JNIEnv *env, jobject instance);

    static void java_SetMirrorMode(JNIEnv *env, jobject instance, jint mode);

    static jint java_GetMirrorMode(JNIEnv *env, jobject instance);

    static void java_SetSpeed(JNIEnv *env, jobject instance, jfloat speed);

    static jfloat java_GetSpeed(JNIEnv *env, jobject instance);

    static void java_SetTraceID(JNIEnv *env, jobject instance, jstring traceId);

    static void java_SetLibPath(JNIEnv *env, jobject instance, jstring libPath);

    static void java_SetOption(JNIEnv *env, jobject instance, jstring key, jstring value);

    static void java_SetAutoPlay(JNIEnv *env, jobject instance,
                                 jboolean autoPlay);

    static jboolean java_IsAutoPlay(JNIEnv *env, jobject instance);

    static void java_SnapShot(JNIEnv *env, jobject instance);


    static jstring java_GetSdkVersion(JNIEnv *env, jclass jclazz);

    static void java_SetBlackType(JNIEnv *env, jclass jclazz, jint type);

    static jstring java_GetCacheFilePathByURL(JNIEnv *env, jobject instance, jstring URL);

    static void java_SetDefaultBandWidth(JNIEnv *env , jobject instance , jint defaultBandWidth);
public:
    static void jni_onError(int64_t code, const void *msg, /*void *extra, */void *userData);

    static void jni_onEvent(int64_t code, const void *msg, /*void *extra, */void *userData);

    static void jni_onPrepared(void *userData);

    static void jni_onCompletion(void *userData);

    static void jni_onCircleStart(void *userData);

    static void jni_onAutoPlayStart(void *userData);

    static void jni_onFirstFrameShow(void *userData);

    static void jni_onVideoSizeChanged(int64_t width, int64_t height, void *userData);

    static void jni_onCurrentPositionUpdate(int64_t position, void *userData);

    static void jni_onBufferPositionUpdate(int64_t position, void *userData);

    static void jni_onLoadingStart(void *userData);

    static void jni_onLoadingProgress(int64_t percent, void *userData);

    static void jni_onLoadingEnd(void *userData);

    static void jni_onSeekEnd(int64_t seekInCache, void *userData);

    static void jni_onStreamInfoGet(int64_t count, const void *infos, void *userData);

    static void
    jni_onSwitchStreamSuccess(int64_t type, const void *item, void *userData);

    static void
    jni_onPlayerStatusChanged(int64_t oldStatus, int64_t newStatus, void *userData);

    static void
    jni_onShowSubtitle(int64_t id, int64_t size, const void *content,/*void *extra,*/
                       void *userData);

    static void
    jni_onHideSubtitle(int64_t id, int64_t size, const void *content, void *userData);

    static void
    jni_onCaptureScreen(int64_t width, int64_t height, const void *buffer,/*void *extra,*/
                        void *userData);

    static void jni_onSubTitleExtAdd(int64_t index, const void *url, void *userData);

private:
    static int64_t mapStatus(int64_t status);
};


#endif //SOURCE_NAITVEBASE_H
