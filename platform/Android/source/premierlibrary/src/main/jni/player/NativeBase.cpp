//
// Created by lifujun on 2019/1/7.
//

#include <cassert>

#include <android/native_window.h>
#include <android/native_window_jni.h>

#include <utils/CicadaDynamicLoader.h>
#include <utils/frame_work_log.h>
#include <utils/af_string.h>
#include <utils/property.h>
#include <native_cicada_player_def.h>
#include <ErrorCodeMap.h>
#include <EventCodeMap.h>
#include <utils/Android/JniEnv.h>
#include <utils/Android/NewStringUTF.h>
#include <utils/Android/GetStringUTFChars.h>
#include <utils/Android/JniException.h>
#include <utils/Android/FindClass.h>


#include "NativeBase.h"
#include "JavaPlayerConfig.h"
#include "JavaCacheConfig.h"
#include "JavaTrackInfo.h"
#include "JavaMediaInfo.h"

#ifdef USE_ARES
    #include "ares.h"
#endif

jclass gj_NativePlayer_Class = nullptr;
jmethodID gj_NativePlayer_onError = nullptr;
jmethodID gj_NativePlayer_onEvent = nullptr;
jmethodID gj_NativePlayer_onSeekEnd = nullptr;
jmethodID gj_NativePlayer_onPrepared = nullptr;
jmethodID gj_NativePlayer_onLoadingEnd = nullptr;
jmethodID gj_NativePlayer_onCompletion = nullptr;
jmethodID gj_NativePlayer_onCircleStart = nullptr;
jmethodID gj_NativePlayer_onLoadingStart = nullptr;
jmethodID gj_NativePlayer_onShowSubtitle = nullptr;
jmethodID gj_NativePlayer_onHideSubtitle = nullptr;
jmethodID gj_NativePlayer_onStatusChanged = nullptr;
jmethodID gj_NativePlayer_onAutoPlayStart = nullptr;
jmethodID gj_NativePlayer_onStreamInfoGet = nullptr;
jmethodID gj_NativePlayer_onCaptureScreen = nullptr;
jmethodID gj_NativePlayer_getNativeContext = nullptr;
jmethodID gj_NativePlayer_setNativeContext = nullptr;
jmethodID gj_NativePlayer_onFirstFrameShow = nullptr;
jmethodID gj_NativePlayer_onLoadingProgress = nullptr;
jmethodID gj_NativePlayer_onSwitchStreamFail = nullptr;
jmethodID gj_NativePlayer_onVideoSizeChanged = nullptr;
jmethodID gj_NativePlayer_onSwitchStreamSuccess = nullptr;
jmethodID gj_NativePlayer_onBufferPositionUpdate = nullptr;
jmethodID gj_NativePlayer_onCurrentPositionUpdate = nullptr;
jmethodID gj_NativePlayer_onSubtitleExtAdded = nullptr;

void NativeBase::java_Construct(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    PlayerPrivateData *privateData = new PlayerPrivateData();
    privateData->j_instance = env->NewGlobalRef(instance);
    privateData->player = new MediaPlayer();
    env->CallVoidMethod(instance, gj_NativePlayer_setNativeContext, (jlong) privateData);
    JniException::clearException(env);
    jobject userData = privateData->j_instance;
    playerListener listener{nullptr};
    listener.userData = userData;
    listener.LoopingStart = jni_onCircleStart;
    listener.AutoPlayStart = jni_onAutoPlayStart;
    listener.FirstFrameShow = jni_onFirstFrameShow;
    listener.VideoSizeChanged = jni_onVideoSizeChanged;
    listener.PositionUpdate = jni_onCurrentPositionUpdate;
    listener.BufferPositionUpdate = jni_onBufferPositionUpdate;
    listener.LoadingStart = jni_onLoadingStart;
    listener.LoadingProgress = jni_onLoadingProgress;
    listener.LoadingEnd = jni_onLoadingEnd;
    listener.SeekEnd = jni_onSeekEnd;
    listener.StreamInfoGet = jni_onStreamInfoGet;
    listener.StreamSwitchSuc = jni_onSwitchStreamSuccess;
    listener.StatusChanged = jni_onPlayerStatusChanged;
    listener.CaptureScreen = jni_onCaptureScreen;
    listener.SubtitleShow = jni_onShowSubtitle;
    listener.SubtitleHide = jni_onHideSubtitle;
    listener.EventCallback = jni_onEvent;
    listener.ErrorCallback = jni_onError;
    listener.Prepared = jni_onPrepared;
    listener.Completion = jni_onCompletion;
    listener.SubtitleExtAdd = jni_onSubTitleExtAdd;
    auto *apsaraPlayer = privateData->player;
    apsaraPlayer->SetListener(listener);
}

void NativeBase::java_SetConnectivityManager(JNIEnv *env, jobject instance, jobject connectManager)
{
    if (connectManager != nullptr) {
#ifdef USE_ARES
        ares_library_init_android(connectManager);
#endif
    }
}

inline PlayerPrivateData *getPlayerPrivateData(JNIEnv *pEnv, jobject pJobject)
{
    jlong privatgeDataAddr = pEnv->CallLongMethod(pJobject,
                             gj_NativePlayer_getNativeContext);
    PlayerPrivateData *privateData = (PlayerPrivateData *) ((long) privatgeDataAddr);
    JniException::clearException(pEnv);
    return privateData;
}

MediaPlayer *NativeBase::getPlayer(JNIEnv *pEnv, jobject pJobject)
{
    PlayerPrivateData *privateData = getPlayerPrivateData(pEnv, pJobject);

    if (privateData == nullptr) {
        return nullptr;
    }

    return privateData->player;
}

void NativeBase::java_SetSpeed(JNIEnv *env, jobject instance, jfloat speed)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    player->SetSpeed(speed);
}

void NativeBase::java_EnableHardwareDecoder(JNIEnv *env, jobject instance,
        jboolean enabled)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    player->EnableHardwareDecoder(enabled);
}


void NativeBase::java_SetView(JNIEnv *env, jobject instance, jobject surface)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    PlayerPrivateData *privateData = getPlayerPrivateData(env, instance);
    jobject mSurfaceRef = env->NewGlobalRef((jobject) surface);;
    player->SetView(mSurfaceRef);

    if (privateData->mSurfaceRef) {
        env->DeleteGlobalRef(privateData->mSurfaceRef);
    }

    privateData->mSurfaceRef = mSurfaceRef;
    JniException::clearException(env);
}


void NativeBase::java_SelectTrack(JNIEnv *env, jobject instance, jint index)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    player->SelectTrack(index);
}

void NativeBase::java_Prepare(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    player->Prepare();
}


void NativeBase::java_Start(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    player->Start();
}


void NativeBase::java_Pause(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    player->Pause();
}


void NativeBase::java_SetVolume(JNIEnv *env, jobject instance, jfloat volume)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    player->SetVolume(volume);
}


jfloat NativeBase::java_GetVolume(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return 0;
    }

    return player->GetVolume();
}


void NativeBase::java_SeekTo(JNIEnv *env, jobject instance, jlong position, jint mode)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    player->SeekTo(position, (SeekMode) mode);
}

void NativeBase::java_SetMaxAccurateSeekDelta(JNIEnv *env, jobject instance, jint delta)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    player->SetOption("maxAccurateSeekDelta", AfString::to_string((int)delta).c_str());
}


void NativeBase::java_Stop(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    player->Stop();
}


void NativeBase::java_Release(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player != nullptr) {
        player->Stop();
        delete player;
        player = nullptr;
    }

    PlayerPrivateData *privateData = getPlayerPrivateData(env, instance);

    if (privateData != nullptr) {
        ANativeWindow *nativeWindow = privateData->nativeWindow;

        if (nativeWindow != nullptr) {
            //     ANativeWindow_release(nativeWindow);
        }

        if (privateData->mSurfaceRef) {
            env->DeleteGlobalRef(privateData->mSurfaceRef);
        }

        jobject jinstance = privateData->j_instance;

        if (jinstance != nullptr) {
            env->DeleteGlobalRef(jinstance);
            jinstance = nullptr;
        }

        delete privateData;
        privateData = nullptr;
    }

    env->CallVoidMethod(instance, gj_NativePlayer_setNativeContext, (jlong) 0);
    JniException::clearException(env);
}


jlong NativeBase::java_GetDuration(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return 0;
    }

    return player->GetDuration();
}

jobject NativeBase::java_GetCurrentStreamInfo(JNIEnv *env, jobject instance, int streamType)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return nullptr;
    }

    StreamInfo *streamInfo = player->GetCurrentStreamInfo(static_cast<StreamType>(streamType));

    if (streamInfo == nullptr) {
        return nullptr;
    } else {
        jobject jStreamInfo = JavaTrackInfo::getTrackInfo(env, *streamInfo);
        return jStreamInfo;
    }
}


jlong NativeBase::java_GetCurrentPosition(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return 0;
    }

    return player->GetCurrentPosition();
}


jlong NativeBase::java_GetBufferedPosition(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return 0;
    }

    return player->GetBufferedPosition();
}


void NativeBase::java_SetMute(JNIEnv *env, jobject instance, jboolean mute)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    player->SetMute(mute);
}


jboolean NativeBase::java_IsMuted(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return (jboolean) false;
    }

    return (jboolean) player->IsMuted();
}


void NativeBase::java_SetConfig(JNIEnv *env, jobject instance, jobject playerConfig)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    if (playerConfig == nullptr) {
        return;
    }

    const MediaPlayerConfig config = JavaPlayerConfig::convertTo(env, playerConfig);
    player->SetConfig(&config);
}


void NativeBase::java_SetCacheConfig(JNIEnv *env, jobject instance, jobject cacheConfig)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    if (cacheConfig == nullptr) {
        return;
    }

    CacheConfig config;
    JavaCacheConfig::convertTo(config, env, cacheConfig);
    player->SetCacheConfig(config);
}

jobject NativeBase::java_GetConfig(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return nullptr;
    }

    const MediaPlayerConfig *playerConfig = player->GetConfig();
    jobject jConfigInfo = JavaPlayerConfig::getJPlayerConfig(env, playerConfig);
    return jConfigInfo;
}


void NativeBase::java_SetScaleMode(JNIEnv *env, jobject instance, jint mode)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    player->SetScaleMode((ScaleMode) mode);
}


jint NativeBase::java_GetScaleMode(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return 0;
    }

    return player->GetScaleMode();
}


void NativeBase::java_SetLoop(JNIEnv *env, jobject instance, jboolean on)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    player->SetLoop(on);
}


jboolean NativeBase::java_IsLoop(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return (jboolean) false;
    }

    return static_cast<jboolean>(player->IsLoop());
}

jint NativeBase::java_GetVideoWidth(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return 0;
    }

    return player->GetVideoWidth();
}


jint NativeBase::java_GetVideoHeight(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return 0;
    }

    return player->GetVideoHeight();
}


jint NativeBase::java_GetVideoRotation(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return 0;
    }

    return player->GetVideoRotation();
}


void NativeBase::java_Reload(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    player->Reload();
}


void NativeBase::java_SetRotateMode(JNIEnv *env, jobject instance, jint mode)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    player->SetRotateMode((RotateMode) mode);
}


jint NativeBase::java_GetRotateMode(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return RotateMode::ROTATE_MODE_0;
    }

    return player->GetRotateMode();
}


void NativeBase::java_SetMirrorMode(JNIEnv *env, jobject instance, jint mode)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    player->SetMirrorMode((MirrorMode) mode);
}


jint NativeBase::java_GetMirrorMode(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return MirrorMode::MIRROR_MODE_NONE;
    }

    return player->GetMirrorMode();
}


jfloat NativeBase::java_GetSpeed(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return 0;
    }

    return player->GetSpeed();
}


void NativeBase::java_SetTraceID(JNIEnv *env, jobject instance, jstring traceId)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    if (traceId == nullptr) {
        AF_TRACE;
        return;
    }

    GetStringUTFChars tmpTraceId(env, traceId);
    char *traceIdStr = tmpTraceId.getChars();
    player->SetTraceID(traceIdStr);
}

void NativeBase::java_SetLibPath(JNIEnv *env, jobject instance, jstring libPath)
{
    if (libPath == nullptr) {
        AF_TRACE;
        return;
    }

    GetStringUTFChars libPathObj(env, libPath);
    char *libPathStr = libPathObj.getChars();
    CicadaDynamicLoader::setWorkPath(libPathStr);
}

void NativeBase::java_SetOption(JNIEnv *env, jobject instance, jstring key, jstring value)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    if (nullptr == key || nullptr == value) {
        AF_TRACE;
        return;
    }

    GetStringUTFChars tmpKey(env, key);
    char *keyStr = tmpKey.getChars();
    GetStringUTFChars tmpValue(env, value);
    char *valueStr = tmpValue.getChars();
    player->SetOption(keyStr, valueStr);
}

void NativeBase::java_SetAutoPlay(JNIEnv *env, jobject instance,
                                  jboolean autoPlay)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return;
    }

    player->SetAutoPlay((bool) autoPlay);
}


jboolean NativeBase::java_IsAutoPlay(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player == nullptr) {
        return (jboolean) false;
    }

    return (jboolean) player->IsAutoPlay();
}


void NativeBase::java_SetDataSource(JNIEnv *env, jobject instance, jstring urlSource)
{
    AF_TRACE;

    if (urlSource == nullptr) {
        return;
    }

    MediaPlayer *player = getPlayer(env, instance);

    if (player != nullptr) {
        GetStringUTFChars tmpUrl(env, urlSource);
        char *chUrl = tmpUrl.getChars();
        player->SetDataSource(chUrl);
    }
}

void NativeBase::java_AddExtSubtitle(JNIEnv *env, jobject instance, jstring url)
{
    if (url == nullptr) {
        return;
    }

    MediaPlayer *player = getPlayer(env, instance);

    if (player != nullptr) {
        GetStringUTFChars tmpUrl(env, url);
        player->AddExtSubtitle(tmpUrl.getChars());
    }
}


void NativeBase::java_SelectExtSubtitle(JNIEnv *env, jobject instance, jint index, jboolean select)
{
    if (index < 0) {
        return;
    }

    MediaPlayer *player = getPlayer(env, instance);

    if (player != nullptr) {
        player->SelectExtSubtitle(index, select);
    }
}

void NativeBase::java_SnapShot(JNIEnv *env, jobject instance)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player != nullptr) {
        player->CaptureScreen();
    }
}

jstring NativeBase::java_GetSdkVersion(JNIEnv *env, jclass jclazz)
{
    AF_TRACE;
    string sdkVersion = MediaPlayer::GetSdkVersion();
    jstring jSdkVersion = env->NewStringUTF(sdkVersion.c_str());
    JniException::clearException(env);
    return jSdkVersion;
}

void NativeBase::java_SetBlackType(JNIEnv *env, jclass jclazz, jint type)
{
    AF_TRACE;

    if (type == 0) { //hw_decode_h264
        setProperty("ro.video.dec.h264", "OFF");
    }
}


jstring NativeBase::java_GetCacheFilePathByURL(JNIEnv *env, jobject instance, jstring URL)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player != nullptr) {
        GetStringUTFChars tmpURL(env, URL);
        char *ch_URL = tmpURL.getChars();
        string fileName = player->GetCachePathByURL(
                              ch_URL == nullptr ? "" : string(ch_URL));

        if (fileName.empty()) {
            return nullptr;
        } else {
            return env->NewStringUTF(fileName.c_str());
        }
    }

    return nullptr;
}


void NativeBase::java_SetDefaultBandWidth(JNIEnv *env, jobject instance, jint defaultBandWidth)
{
    AF_TRACE;
    MediaPlayer *player = getPlayer(env, instance);

    if (player != nullptr) {
        player->SetDefaultBandWidth((int)defaultBandWidth);
    }
}

//callback...

void NativeBase::init(JNIEnv *env)
{
    if (gj_NativePlayer_Class == nullptr) {
        FindClass cls(env, nativePlayerPath);
        gj_NativePlayer_Class = (jclass) env->NewGlobalRef(cls.getClass());
        gj_NativePlayer_onError = env->GetMethodID(gj_NativePlayer_Class,
                                  "onError",
                                  "(ILjava/lang/String;Ljava/lang/Object;)V");
        gj_NativePlayer_onEvent = env->GetMethodID(gj_NativePlayer_Class,
                                  "onEvent",
                                  "(ILjava/lang/String;Ljava/lang/Object;)V");
        gj_NativePlayer_onSeekEnd = env->GetMethodID(gj_NativePlayer_Class,
                                    "onSeekEnd",
                                    "()V");
        gj_NativePlayer_onPrepared = env->GetMethodID(gj_NativePlayer_Class,
                                     "onPrepared",
                                     "()V");
        gj_NativePlayer_onLoadingEnd = env->GetMethodID(gj_NativePlayer_Class,
                                       "onLoadingEnd",
                                       "()V");
        gj_NativePlayer_onCompletion = env->GetMethodID(gj_NativePlayer_Class,
                                       "onCompletion",
                                       "()V");
        gj_NativePlayer_onCircleStart = env->GetMethodID(gj_NativePlayer_Class,
                                        "onCircleStart",
                                        "()V");
        gj_NativePlayer_onShowSubtitle = env->GetMethodID(gj_NativePlayer_Class,
                                         "onShowSubtitle",
                                         "(IJLjava/lang/String;Ljava/lang/Object;)V");
        gj_NativePlayer_onLoadingStart = env->GetMethodID(gj_NativePlayer_Class,
                                         "onLoadingStart",
                                         "()V");
        gj_NativePlayer_onHideSubtitle = env->GetMethodID(gj_NativePlayer_Class,
                                         "onHideSubtitle",
                                         "(IJ)V");
        gj_NativePlayer_onStatusChanged = env->GetMethodID(gj_NativePlayer_Class,
                                          "onStatusChanged",
                                          "(II)V");
        gj_NativePlayer_onStreamInfoGet = env->GetMethodID(gj_NativePlayer_Class,
                                          "onStreamInfoGet",
                                          "(Lcom/cicada/player/nativeclass/MediaInfo;)V");
        gj_NativePlayer_setNativeContext = env->GetMethodID(gj_NativePlayer_Class,
                                           "setNativeContext",
                                           "(J)V");
        gj_NativePlayer_getNativeContext = env->GetMethodID(gj_NativePlayer_Class,
                                           "getNativeContext",
                                           "()J");
        gj_NativePlayer_onFirstFrameShow = env->GetMethodID(gj_NativePlayer_Class,
                                           "onFirstFrameShow",
                                           "()V");
        gj_NativePlayer_onLoadingProgress = env->GetMethodID(gj_NativePlayer_Class,
                                            "onLoadingProgress",
                                            "(F)V");
        gj_NativePlayer_onSwitchStreamFail = env->GetMethodID(gj_NativePlayer_Class,
                                             "onSwitchStreamFail",
                                             "(Lcom/cicada/player/nativeclass/TrackInfo;ILjava/lang/String;)V");
        gj_NativePlayer_onVideoSizeChanged = env->GetMethodID(gj_NativePlayer_Class,
                                             "onVideoSizeChanged",
                                             "(II)V");
        gj_NativePlayer_onSwitchStreamSuccess = env->GetMethodID(gj_NativePlayer_Class,
                                                "onSwitchStreamSuccess",
                                                "(Lcom/cicada/player/nativeclass/TrackInfo;)V");
        gj_NativePlayer_onBufferPositionUpdate = env->GetMethodID(gj_NativePlayer_Class,
                "onBufferedPositionUpdate",
                "(J)V");
        gj_NativePlayer_onCurrentPositionUpdate = env->GetMethodID(gj_NativePlayer_Class,
                "onCurrentPositionUpdate",
                "(J)V");
        gj_NativePlayer_onCaptureScreen = env->GetMethodID(gj_NativePlayer_Class,
                                          "onCaptureScreen",
                                          "(II[B)V");
        gj_NativePlayer_onAutoPlayStart = env->GetMethodID(gj_NativePlayer_Class,
                                          "onAutoPlayStart",
                                          "()V");
        gj_NativePlayer_onSubtitleExtAdded = env->GetMethodID(gj_NativePlayer_Class,
                                             "onSubtitleExtAdded",
                                             "(ILjava/lang/String;)V");
        JniException::clearException(env);
    }
}

void NativeBase::unInit(JNIEnv *pEnv)
{
    if (gj_NativePlayer_Class != nullptr) {
        pEnv->DeleteGlobalRef(gj_NativePlayer_Class);
        gj_NativePlayer_Class = nullptr;
        JniException::clearException(pEnv);
    }
}

static JNINativeMethod nativePlayer_method_table[] = {
    {"nConstruct",              "()V",                                     (void *) NativeBase::java_Construct},
    {"nSetConnectivityManager", "(Ljava/lang/Object;)V",                   (void *) NativeBase::java_SetConnectivityManager},
    {"nEnableHardwareDecoder",  "(Z)V",                                    (void *) NativeBase::java_EnableHardwareDecoder},
    {"nSetSurface",             "(Landroid/view/Surface;)V",               (void *) NativeBase::java_SetView},
    {"nSetDataSource",          "(Ljava/lang/String;)V",                   (void *) NativeBase::java_SetDataSource},
    {"nAddExtSubtitle",         "(Ljava/lang/String;)V",                   (void *) NativeBase::java_AddExtSubtitle},
    {"nSelectExtSubtitle",      "(IZ)V",                                   (void *) NativeBase::java_SelectExtSubtitle},
    {"nSelectTrack",            "(I)V",                                    (void *) NativeBase::java_SelectTrack},
    {"nPrepare",                "()V",                                     (void *) NativeBase::java_Prepare},
    {"nStart",                  "()V",                                     (void *) NativeBase::java_Start},
    {"nPause",                  "()V",                                     (void *) NativeBase::java_Pause},
    {"nSetVolume",              "(F)V",                                    (void *) NativeBase::java_SetVolume},
    {"nGetVolume",              "()F",                                     (void *) NativeBase::java_GetVolume},
    {"nSeekTo",                 "(JI)V",                                   (void *) NativeBase::java_SeekTo},
    {"nSetMaxAccurateSeekDelta", "(I)V",                                    (void *) NativeBase::java_SetMaxAccurateSeekDelta},
    {"nStop",                   "()V",                                     (void *) NativeBase::java_Stop},
    {"nRelease",                "()V",                                     (void *) NativeBase::java_Release},
    {"nGetDuration",            "()J",                                     (void *) NativeBase::java_GetDuration},
    {"nGetCurrentStreamInfo",   "(I)Ljava/lang/Object;",                   (void *) NativeBase::java_GetCurrentStreamInfo},
    {"nGetCurrentPosition",     "()J",                                     (void *) NativeBase::java_GetCurrentPosition},
    {"nGetBufferedPosition",    "()J",                                     (void *) NativeBase::java_GetBufferedPosition},
    {"nSetMute",                "(Z)V",                                    (void *) NativeBase::java_SetMute},
    {"nIsMuted",                "()Z",                                     (void *) NativeBase::java_IsMuted},
    {"nSetConfig",              "(Ljava/lang/Object;)V",                   (void *) NativeBase::java_SetConfig},
    {"nGetConfig",              "()Ljava/lang/Object;",                    (void *) NativeBase::java_GetConfig},
    {"nSetCacheConfig",         "(Ljava/lang/Object;)V",                   (void *) NativeBase::java_SetCacheConfig},
    {"nSetScaleMode",           "(I)V",                                    (void *) NativeBase::java_SetScaleMode},
    {"nGetScaleMode",           "()I",                                     (void *) NativeBase::java_GetScaleMode},
    {"nSetLoop",                "(Z)V",                                    (void *) NativeBase::java_SetLoop},
    {"nIsLoop",                 "()Z",                                     (void *) NativeBase::java_IsLoop},
    {"nGetVideoWidth",          "()I",                                     (void *) NativeBase::java_GetVideoWidth},
    {"nGetVideoHeight",         "()I",                                     (void *) NativeBase::java_GetVideoHeight},
    {"nGetVideoRotation",       "()I",                                     (void *) NativeBase::java_GetVideoRotation},
    {"nReload",                 "()V",                                     (void *) NativeBase::java_Reload},
    {"nSetRotateMode",          "(I)V",                                    (void *) NativeBase::java_SetRotateMode},
    {"nGetRotateMode",          "()I",                                     (void *) NativeBase::java_GetRotateMode},
    {"nSetMirrorMode",          "(I)V",                                    (void *) NativeBase::java_SetMirrorMode},
    {"nGetMirrorMode",          "()I",                                     (void *) NativeBase::java_GetMirrorMode},
    {"nSetSpeed",               "(F)V",                                    (void *) NativeBase::java_SetSpeed},
    {"nGetSpeed",               "()F",                                     (void *) NativeBase::java_GetSpeed},
    {"nSetTraceID",             "(Ljava/lang/String;)V",                   (void *) NativeBase::java_SetTraceID},
    {"nSetLibPath",             "(Ljava/lang/String;)V",                   (void *) NativeBase::java_SetLibPath},
    {"nSetOption",              "(Ljava/lang/String;Ljava/lang/String;)V", (void *) NativeBase::java_SetOption},
//        {"nSetComponentCb",        "(IJJJ)V",                                 (void *) NativeBase::java_SetComponentCb},
    {"nSetAutoPlay",            "(Z)V",                                    (void *) NativeBase::java_SetAutoPlay},
    {"nIsAutoPlay",             "()Z",                                     (void *) NativeBase::java_IsAutoPlay},
    {"nSnapShot",               "()V",                                     (void *) NativeBase::java_SnapShot},
    {"nGetSdkVersion",          "()Ljava/lang/String;",                    (void *) NativeBase::java_GetSdkVersion},
    {"nSetBlackType",           "(I)V",                                    (void *) NativeBase::java_SetBlackType},
    {"nGetCacheFilePath",       "(Ljava/lang/String;)Ljava/lang/String;",  (void *) NativeBase::java_GetCacheFilePathByURL},
    {"nSetDefaultBandWidth",    "(I)V",                                                                        (void *) NativeBase::java_SetDefaultBandWidth},

};


int NativeBase::registerMethod(JNIEnv *pEnv)
{
    if (gj_NativePlayer_Class == nullptr) {
        return JNI_FALSE;
    }

    if (pEnv->RegisterNatives(gj_NativePlayer_Class, nativePlayer_method_table,
                              sizeof(nativePlayer_method_table) / sizeof(JNINativeMethod)) < 0) {
        JniException::clearException(pEnv);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

int64_t NativeBase::mapStatus(int64_t status)
{
    if (status == PLAYER_IDLE) {
        return 0;
    } else if (status == PLAYER_INITIALZED) {
        return 1;
    } else if (status == PLAYER_PREPARED) {
        return 2;
    } else if (status == PLAYER_PLAYING) {
        return 3;
    } else if (status == PLAYER_PAUSED) {
        return 4;
    } else if (status == PLAYER_STOPPED) {
        return 5;
    } else if (status == PLAYER_COMPLETION) {
        return 6;
    } else if (status == PLAYER_ERROR) {
        return 7;
    } else {
        return -1;
    }
}

void NativeBase::jni_onCircleStart(void *userData)
{
    if (userData == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    mEnv->CallVoidMethod((jobject) userData, gj_NativePlayer_onCircleStart);
    JniException::clearException(mEnv);
}

void NativeBase::jni_onAutoPlayStart(void *userData)
{
    if (userData == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    mEnv->CallVoidMethod((jobject) userData, gj_NativePlayer_onAutoPlayStart);
    JniException::clearException(mEnv);
}

void NativeBase::jni_onFirstFrameShow(void *userData)
{
    if (userData == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    mEnv->CallVoidMethod((jobject) userData, gj_NativePlayer_onFirstFrameShow);
    JniException::clearException(mEnv);
}

void NativeBase::jni_onVideoSizeChanged(int64_t width, int64_t height, void *userData)
{
    AF_LOGI("AliyunCorePlayer jni_onVideoSizeChanged  width = %d ,height = %d", width, height);

    if (userData == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    int finalWith = static_cast<int>(width);
    int finalHeight = static_cast<int>(height);
    mEnv->CallVoidMethod((jobject) userData, gj_NativePlayer_onVideoSizeChanged,
                         (jint) finalWith,
                         (jint) finalHeight);
    JniException::clearException(mEnv);
}

void NativeBase::jni_onCurrentPositionUpdate(int64_t position, void *userData)
{
    if (userData == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    mEnv->CallVoidMethod((jobject) userData, gj_NativePlayer_onCurrentPositionUpdate,
                         (jlong) position);
    JniException::clearException(mEnv);
}

void NativeBase::jni_onBufferPositionUpdate(int64_t position, void *userData)
{
    if (userData == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    mEnv->CallVoidMethod((jobject) userData, gj_NativePlayer_onBufferPositionUpdate,
                         (jlong) position);
    JniException::clearException(mEnv);
}

void NativeBase::jni_onLoadingStart(void *userData)
{
    if (userData == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    mEnv->CallVoidMethod((jobject) userData, gj_NativePlayer_onLoadingStart);
    JniException::clearException(mEnv);
}

void NativeBase::jni_onLoadingProgress(int64_t percent, void *userData)
{
    if (userData == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    mEnv->CallVoidMethod((jobject) userData, gj_NativePlayer_onLoadingProgress,
                         (jfloat) percent);
    JniException::clearException(mEnv);
}

void NativeBase::jni_onLoadingEnd(void *userData)
{
    if (userData == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    mEnv->CallVoidMethod((jobject) userData, gj_NativePlayer_onLoadingEnd);
    JniException::clearException(mEnv);
}

void NativeBase::jni_onSeekEnd(int64_t seekInCache, void *userData)
{
    if (userData == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    mEnv->CallVoidMethod((jobject) userData, gj_NativePlayer_onSeekEnd, seekInCache != 0);
    JniException::clearException(mEnv);
}

void NativeBase::jni_onPlayerStatusChanged(int64_t oldStatus, int64_t newStatus, void *userData)
{
    if (userData == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    int64_t javaOldStatus = mapStatus(oldStatus);
    int64_t javaNewStatus = mapStatus(newStatus);;

    if (javaNewStatus < 0) {
        return;
    }

    mEnv->CallVoidMethod((jobject) userData, gj_NativePlayer_onStatusChanged,
                         (jint) javaNewStatus,
                         (jint) javaOldStatus);
    JniException::clearException(mEnv);
}

void NativeBase::jni_onCaptureScreen(int64_t width, int64_t height, const void *buffer, void *userData)
{
    if (userData == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    jbyteArray buf = nullptr;

    if (buffer != nullptr) {
        int64_t size = width * height * 4;//RGBA_8888
        buf = mEnv->NewByteArray(size);

        if (buf != nullptr) {
            mEnv->SetByteArrayRegion(buf, 0, size, (jbyte *) (buffer));
        }
    }

    mEnv->CallVoidMethod(static_cast<jobject>(userData), gj_NativePlayer_onCaptureScreen,
                         (jint) width, (jint) height,
                         buf);

    if (buf != nullptr) {
        mEnv->DeleteLocalRef(buf);
    }

    JniException::clearException(mEnv);
}

void NativeBase::jni_onSubTitleExtAdd(int64_t index, const void *url, void *userData)
{
    if (userData == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *pEnv = Jenv.getEnv();

    if (pEnv == nullptr) {
        return;
    }

    NewStringUTF tmpmsg(pEnv, (char *) url);
    jstring jmsg = tmpmsg.getString();
    pEnv->CallVoidMethod((jobject) userData, gj_NativePlayer_onSubtitleExtAdded, (jint) index,
                         jmsg);
    JniException::clearException(pEnv);
}

void NativeBase::jni_onShowSubtitle(int64_t id, int64_t size, const void *content, void *userData)
{
    if (userData == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *pEnv = Jenv.getEnv();

    if (pEnv == nullptr) {
        return;
    }

    IAFPacket *packet = (IAFPacket *) (content);
//TODO: append all string
    char *srcContent = static_cast<char *>(malloc(static_cast<size_t>(packet->getSize() + 1)));
    memcpy(srcContent, packet->getData(), static_cast<size_t>(packet->getSize()));
    srcContent[packet->getSize()] = 0;
    NewStringUTF tmpContent(pEnv, srcContent);
    jstring jContent = tmpContent.getString();
    pEnv->CallVoidMethod((jobject) userData, gj_NativePlayer_onShowSubtitle,
                         (jint) packet->getInfo().streamIndex, (jlong) packet->getInfo().pts,
                         jContent,
                         nullptr);
    JniException::clearException(pEnv);
    free(srcContent);
}

void NativeBase::jni_onHideSubtitle(int64_t id, int64_t size, const void *content, void *userData)
{
    if (userData == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    IAFPacket *packet = (IAFPacket *) (content);
    mEnv->CallVoidMethod((jobject) userData, gj_NativePlayer_onHideSubtitle,
                         (jint) packet->getInfo().streamIndex,
                         (jlong) packet->getInfo().pts);
    JniException::clearException(mEnv);
}

void NativeBase::jni_onEvent(int64_t code, const void *msg, void *userData)
{
    if (userData == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    NewStringUTF tmpmsg(mEnv, (char *) msg);
    jstring jmsg = tmpmsg.getString();
    jint eventCode = EventCodeMap::getInstance()->getValue(code);
    mEnv->CallVoidMethod((jobject) userData, gj_NativePlayer_onEvent, (jint) eventCode, jmsg,
                         nullptr);
    JniException::clearException(mEnv);
}

void NativeBase::jni_onError(int64_t code, const void *msg, void *userData)
{
    if (userData == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    NewStringUTF tmpmsg(mEnv, (char *) msg);
    jstring jmsg = tmpmsg.getString();
    int javaValue = ErrorCodeMap::getInstance()->getValue(code);
    mEnv->CallVoidMethod((jobject) userData, gj_NativePlayer_onError, (jint) javaValue, jmsg,
                         nullptr);
    JniException::clearException(mEnv);
}

void NativeBase::jni_onPrepared(void *userData)
{
    if (userData == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    mEnv->CallVoidMethod((jobject) userData, gj_NativePlayer_onPrepared);
    JniException::clearException(mEnv);
}

void NativeBase::jni_onCompletion(void *userData)
{
    if (userData == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    mEnv->CallVoidMethod((jobject) userData, gj_NativePlayer_onCompletion);
    JniException::clearException(mEnv);
}

void NativeBase::jni_onStreamInfoGet(int64_t count, const void *infos, void *userData)
{
    if (userData == nullptr || count == 0 || infos == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    jobject jMediaInfo = JavaMediaInfo::convertTo(mEnv, infos, count);
    mEnv->CallVoidMethod((jobject) userData, gj_NativePlayer_onStreamInfoGet, jMediaInfo);
    mEnv->DeleteLocalRef(jMediaInfo);
    JniException::clearException(mEnv);
}

void NativeBase::jni_onSwitchStreamSuccess(int64_t type, const void *item, void *userData)
{
    if (userData == nullptr || item == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    const auto *newStream = static_cast<const StreamInfo *>(item);
    jobject jStreamInfoNew = JavaTrackInfo::getTrackInfo(mEnv, *newStream);
    mEnv->CallVoidMethod((jobject) userData, gj_NativePlayer_onSwitchStreamSuccess,
                         jStreamInfoNew);
    mEnv->DeleteLocalRef(jStreamInfoNew);
    JniException::clearException(mEnv);
}
