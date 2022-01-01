//
// Created by lifujun on 2020/7/23.
//

#include <utils/Android/FindClass.h>
#include <utils/Android/JniEnv.h>
#include <utils/Android/NewStringUTF.h>
#include <utils/Android/JniException.h>
#include <utils/Android/GetStringUTFChars.h>
#include <base/media/subTitlePacket.h>
#include <utils/Android/JniUtils.h>
#include <utils/CicadaJSON.h>
#include <utils/CicadaUtils.h>
#include "JavaExternalPlayer.h"
#include "JavaOptions.h"
#include "JavaMediaInfo.h"
#include "JavaTrackInfo.h"


JavaExternalPlayer JavaExternalPlayer::se(1);

static char *externalPlayerPath = (char *) ("com/cicada/player/nativeclass/NativeExternalPlayer");

static jclass gj_NativeExternalPlayer_Class = nullptr;
static jmethodID gj_NativeExternalPlayer_init = nullptr;
static jmethodID gj_NativeExternalPlayer_isSupport = nullptr;
static jmethodID gj_NativeExternalPlayer_create = nullptr;


static jmethodID gj_NativeExternalPlayer_callRvPv = nullptr;
static jmethodID gj_NativeExternalPlayer_callRvPf = nullptr;
static jmethodID gj_NativeExternalPlayer_callRvPi = nullptr;
static jmethodID gj_NativeExternalPlayer_callRvPo = nullptr;
static jmethodID gj_NativeExternalPlayer_callRvPs = nullptr;
static jmethodID gj_NativeExternalPlayer_callRvPlb = nullptr;
static jmethodID gj_NativeExternalPlayer_callRfPvD = nullptr;
static jmethodID gj_NativeExternalPlayer_callRlPvD = nullptr;
static jmethodID gj_NativeExternalPlayer_callRiPvD = nullptr;
static jmethodID gj_NativeExternalPlayer_callRbPvD = nullptr;
static jmethodID gj_NativeExternalPlayer_callRiPiD = nullptr;
static jmethodID gj_NativeExternalPlayer_callRoPi = nullptr;


void JavaExternalPlayer::init(JNIEnv *env) {
    if (gj_NativeExternalPlayer_Class == nullptr) {
        FindClass cls(env, externalPlayerPath);
        gj_NativeExternalPlayer_Class = (jclass) env->NewGlobalRef(cls.getClass());
        gj_NativeExternalPlayer_init = env->GetMethodID(gj_NativeExternalPlayer_Class, "<init>",
                                                        "()V");
        gj_NativeExternalPlayer_isSupport = env->GetStaticMethodID(gj_NativeExternalPlayer_Class,
                                                                   "isSupport",
                                                                   "(Lcom/cicada/player/nativeclass/Options;)Z");
        gj_NativeExternalPlayer_create = env->GetMethodID(gj_NativeExternalPlayer_Class, "create",
                                                          "(JLcom/cicada/player/nativeclass/Options;)V");


        gj_NativeExternalPlayer_callRvPv = env->GetMethodID(gj_NativeExternalPlayer_Class,
                                                            "callRvPv",
                                                            "(Ljava/lang/String;)V");
        gj_NativeExternalPlayer_callRvPf = env->GetMethodID(gj_NativeExternalPlayer_Class,
                                                            "callRvPf",
                                                            "(Ljava/lang/String;F)V");
        gj_NativeExternalPlayer_callRvPi = env->GetMethodID(gj_NativeExternalPlayer_Class,
                                                            "callRvPi",
                                                            "(Ljava/lang/String;I)V");
        gj_NativeExternalPlayer_callRvPo = env->GetMethodID(gj_NativeExternalPlayer_Class,
                                                            "callRvPo",
                                                            "(Ljava/lang/String;Ljava/lang/Object;)V");
        gj_NativeExternalPlayer_callRvPs = env->GetMethodID(gj_NativeExternalPlayer_Class,
                                                            "callRvPs",
                                                            "(Ljava/lang/String;Ljava/lang/String;)V");
        gj_NativeExternalPlayer_callRfPvD = env->GetMethodID(gj_NativeExternalPlayer_Class,
                                                             "callRfPvD",
                                                             "(Ljava/lang/String;F)F");
        gj_NativeExternalPlayer_callRlPvD = env->GetMethodID(gj_NativeExternalPlayer_Class,
                                                             "callRlPvD",
                                                             "(Ljava/lang/String;J)J");
        gj_NativeExternalPlayer_callRiPvD = env->GetMethodID(gj_NativeExternalPlayer_Class,
                                                             "callRiPvD",
                                                             "(Ljava/lang/String;I)I");
        gj_NativeExternalPlayer_callRvPlb = env->GetMethodID(gj_NativeExternalPlayer_Class,
                                                             "callRvPlb",
                                                             "(Ljava/lang/String;JZ)V");
        gj_NativeExternalPlayer_callRbPvD = env->GetMethodID(gj_NativeExternalPlayer_Class,
                                                             "callRbPvD",
                                                             "(Ljava/lang/String;Z)Z");
        gj_NativeExternalPlayer_callRoPi = env->GetMethodID(gj_NativeExternalPlayer_Class,
                                                            "callRoPi",
                                                            "(Ljava/lang/String;I)Ljava/lang/Object;");
        gj_NativeExternalPlayer_callRiPiD = env->GetMethodID(gj_NativeExternalPlayer_Class,
                                                             "callRiPiD",
                                                             "(Ljava/lang/String;II)I");

    }


}

void JavaExternalPlayer::unInit(JNIEnv *pEnv) {

}

static JNINativeMethod nativePlayer_method_table[] = {
        {"nativeOnPrepared",             "(J)V",                                           (void *) JavaExternalPlayer::java_OnPrepared},
        {"nativeOnLoopingStart",         "(J)V",                                           (void *) JavaExternalPlayer::java_OnLoopingStart},
        {"nativeOnCompletion",           "(J)V",                                           (void *) JavaExternalPlayer::java_OnCompletion},
        {"nativeOnFirstFrameShow",       "(J)V",                                           (void *) JavaExternalPlayer::java_OnFirstFrameShow},
        {"nativeOnLoadingStart",         "(J)V",                                           (void *) JavaExternalPlayer::java_OnLoadingStart},
        {"nativeOnLoadingEnd",           "(J)V",                                           (void *) JavaExternalPlayer::java_OnLoadingEnd},
        {"nativeOnAutoPlayStart",        "(J)V",                                           (void *) JavaExternalPlayer::java_OnAutoPlayStart},

        {"nativeOnSeeking",              "(JZ)V",                                          (void *) JavaExternalPlayer::java_OnSeeking},
        {"nativeOnSeekEnd",              "(JZ)V",                                          (void *) JavaExternalPlayer::java_OnSeekEnd},
        {"nativeOnPositionUpdate",       "(JJ)V",                                          (void *) JavaExternalPlayer::java_OnPositionUpdate},
        {"nativeOnBufferPositionUpdate", "(JJ)V",                                          (void *) JavaExternalPlayer::java_OnBufferPositionUpdate},
        {"nativeOnLoadingProgress",      "(JJ)V",                                          (void *) JavaExternalPlayer::java_OnLoadingProgress},


        {"nativeOnVideoSizeChanged",     "(JII)V",                                         (void *) JavaExternalPlayer::java_OnVideoSizeChanged},
        {"nativeOnStatusChanged",        "(JII)V",                                         (void *) JavaExternalPlayer::java_OnStatusChanged},
        {"nativeOnVideoRendered",        "(JJJ)V",                                         (void *) JavaExternalPlayer::java_OnVideoRendered},


        {"nativeOnErrorCallback",        "(JJLjava/lang/String;)V",                        (void *) JavaExternalPlayer::java_OnErrorCallback},
        {"nativeOnEventCallback",        "(JJLjava/lang/String;)V",                        (void *) JavaExternalPlayer::java_OnEventCallback},
        {"nativeOnStreamInfoGet",        "(JLcom/cicada/player/nativeclass/MediaInfo;)V",  (void *) JavaExternalPlayer::java_OnStreamInfoGet},
        {"nativeOnStreamSwitchSuc",      "(JILcom/cicada/player/nativeclass/TrackInfo;)V", (void *) JavaExternalPlayer::java_OnStreamSwitchSuc},

        {"nativeOnCaptureScreen",        "(JII[B)V",                                       (void *) JavaExternalPlayer::java_OnCaptureScreen},

        {"nativeOnSubtitleHide",         "(JJ[B)V",                                        (void *) JavaExternalPlayer::java_OnSubtitleHide},
        {"nativeOnSubtitleShow",         "(JJ[B)V",                                        (void *) JavaExternalPlayer::java_OnSubtitleShow},
        {"nativeOnSubtitleExtAdd",       "(JJLjava/lang/String;)V",                        (void *) JavaExternalPlayer::java_OnSubtitleExtAdd},

        {"nativeOnRequestProvision",     "(JLjava/lang/String;[B)[B",                      (void *) JavaExternalPlayer::java_OnRequestProvision},
        {"nativeOnRequestKey",           "(JLjava/lang/String;[B)[B",                      (void *) JavaExternalPlayer::java_OnRequestKey},

};


int JavaExternalPlayer::registerMethod(JNIEnv *pEnv) {
    if (gj_NativeExternalPlayer_Class == nullptr) {
        return JNI_FALSE;
    }

    if (pEnv->RegisterNatives(gj_NativeExternalPlayer_Class, nativePlayer_method_table,
                              sizeof(nativePlayer_method_table) / sizeof(JNINativeMethod)) < 0) {
        JniException::clearException(pEnv);
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

//========java call back===========

void JavaExternalPlayer::java_OnPrepared(JNIEnv *pEnv, jobject object, jlong nativeInstance) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        player->mPlayerListener.Prepared(player->mPlayerListener.userData);
    }
}

void JavaExternalPlayer::java_OnLoopingStart(JNIEnv *pEnv, jobject object, jlong nativeInstance) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        player->mPlayerListener.LoopingStart(player->mPlayerListener.userData);
    }
}

void JavaExternalPlayer::java_OnCompletion(JNIEnv *pEnv, jobject object, jlong nativeInstance) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        player->mPlayerListener.Completion(player->mPlayerListener.userData);
    }
}

void JavaExternalPlayer::java_OnFirstFrameShow(JNIEnv *pEnv, jobject object, jlong nativeInstance) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        player->mPlayerListener.FirstFrameShow(player->mPlayerListener.userData);
    }
}

void JavaExternalPlayer::java_OnLoadingStart(JNIEnv *pEnv, jobject object, jlong nativeInstance) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        player->mPlayerListener.LoadingStart(player->mPlayerListener.userData);
    }
}

void JavaExternalPlayer::java_OnLoadingEnd(JNIEnv *pEnv, jobject object, jlong nativeInstance) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        player->mPlayerListener.LoadingEnd(player->mPlayerListener.userData);
    }
}

void JavaExternalPlayer::java_OnAutoPlayStart(JNIEnv *pEnv, jobject object, jlong nativeInstance) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        player->mPlayerListener.AutoPlayStart(player->mPlayerListener.userData);
    }
}

void JavaExternalPlayer::java_OnSeeking(JNIEnv *pEnv, jobject object, jlong nativeInstance,
                                        jboolean seekInCache) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        player->mPlayerListener.Seeking((bool) seekInCache ? 1 : 0,
                                        player->mPlayerListener.userData);
    }
}

void JavaExternalPlayer::java_OnSeekEnd(JNIEnv *pEnv, jobject object, jlong nativeInstance,
                                        jboolean seekInCache) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        player->mPlayerListener.SeekEnd((bool) seekInCache ? 1 : 0,
                                        player->mPlayerListener.userData);
    }
}

void JavaExternalPlayer::java_OnPositionUpdate(JNIEnv *pEnv, jobject object, jlong nativeInstance,
                                               jlong position) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        player->mPlayerListener.PositionUpdate((int64_t) position,
                                               player->mPlayerListener.userData);
    }
}

void
JavaExternalPlayer::java_OnBufferPositionUpdate(JNIEnv *pEnv, jobject object, jlong nativeInstance,
                                                jlong bufferPosition) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        player->mPlayerListener.BufferPositionUpdate((int64_t) bufferPosition,
                                                     player->mPlayerListener.userData);
    }
}

void JavaExternalPlayer::java_OnLoadingProgress(JNIEnv *pEnv, jobject object, jlong nativeInstance,
                                                jlong progress) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        player->mPlayerListener.LoadingProgress((int64_t) progress,
                                                player->mPlayerListener.userData);
    }
}

void JavaExternalPlayer::java_OnVideoSizeChanged(JNIEnv *pEnv, jobject object, jlong nativeInstance,
                                                 jint with, jint height) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        player->mPlayerListener.VideoSizeChanged((int) with, (int) height,
                                                 player->mPlayerListener.userData);
    }
}

void JavaExternalPlayer::java_OnStatusChanged(JNIEnv *pEnv, jobject object, jlong nativeInstance,
                                              jint from, jint to) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        player->mPlayerListener.StatusChanged((int) from, (int) to,
                                              player->mPlayerListener.userData);
    }
}

void JavaExternalPlayer::java_OnVideoRendered(JNIEnv *pEnv, jobject object, jlong nativeInstance,
                                              jlong timeMs, jlong pts) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        player->mPlayerListener.VideoRendered((int64_t) timeMs, (int64_t) pts,
                                              player->mPlayerListener.userData);
    }
}

void JavaExternalPlayer::java_OnErrorCallback(JNIEnv *pEnv, jobject object, jlong nativeInstance,
                                              jlong code, jstring jmsg) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        GetStringUTFChars msg(pEnv, jmsg);
        player->mPlayerListener.ErrorCallback((int64_t) code, msg.getChars(),
                                              player->mPlayerListener.userData);
    }
}

void JavaExternalPlayer::java_OnEventCallback(JNIEnv *pEnv, jobject object, jlong nativeInstance,
                                              jlong code, jstring jmsg) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        GetStringUTFChars msg(pEnv, jmsg);
        player->mPlayerListener.EventCallback((int64_t) code, msg.getChars(),
                                              player->mPlayerListener.userData);
    }
}

void JavaExternalPlayer::java_OnStreamInfoGet(JNIEnv *pEnv, jobject object, jlong nativeInstance,
                                              jobject mediaInfo) {
    if (mediaInfo == nullptr) {
        return;
    }

    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        player->mStreamInfos = JavaMediaInfo::convertToStream(pEnv, mediaInfo,
                                                              &player->mStreamCount);
        player->mPlayerListener.StreamInfoGet(player->mStreamCount, player->mStreamInfos,
                                              player->mPlayerListener.userData);
    }

}

void JavaExternalPlayer::java_OnStreamSwitchSuc(JNIEnv *pEnv, jobject object, jlong nativeInstance,
                                                jint type, jobject trackInfo) {
    if (trackInfo == nullptr) {
        return;
    }

    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        int index = JavaTrackInfo::getStreamIndexByTrackInfo(pEnv, trackInfo);
        StreamInfo *streamInfo = player->getStreamInfoByIndex(index);
        player->mPlayerListener.StreamSwitchSuc((int64_t) type, streamInfo,
                                                player->mPlayerListener.userData);
    }
}

void JavaExternalPlayer::java_OnCaptureScreen(JNIEnv *pEnv, jobject object, jlong nativeInstance,
                                              jint width, jint height, jbyteArray datas) {
    //TODO
}

void JavaExternalPlayer::java_OnSubtitleHide(JNIEnv *pEnv, jobject object, jlong nativeInstance,
                                             jint index, jbyteArray content) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        IAFPacket *packet = new subTitlePacket(nullptr, 0, 0, 0);
        packet->getInfo().streamIndex = index;
        player->mPlayerListener.SubtitleHide(index, sizeof(IAFPacket), (void *) (packet),
                                             player->mPlayerListener.userData);

        delete packet;
    }

}

void JavaExternalPlayer::java_OnSubtitleShow(JNIEnv *pEnv, jobject object, jlong nativeInstance,
                                             jint index, jbyteArray content) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr) {
        char *data = JniUtils::jByteArrayToChars(pEnv, content);
        IAFPacket *packet = new subTitlePacket((uint8_t *) data, strlen(data), 0, 0);
        packet->getInfo().streamIndex = index;
        player->mPlayerListener.SubtitleShow(index, sizeof(IAFPacket), (void *) (packet),
                                             player->mPlayerListener.userData);

        free(data);
        delete packet;
    }
}

void JavaExternalPlayer::java_OnSubtitleExtAdd(JNIEnv *pEnv, jobject object, jlong nativeInstance,
                                               jlong index, jstring url) {
    //TODO
}

jbyteArray
JavaExternalPlayer::java_OnRequestProvision(JNIEnv *pEnv, jobject object, jlong nativeInstance,
                                            jstring provisionUrl, jbyteArray data) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr && player->mDrmCallback != nullptr) {
        GetStringUTFChars cUrl(pEnv, provisionUrl);
        char *cData = JniUtils::jByteArrayToChars(pEnv, data);
        int dataLen = pEnv->GetArrayLength(data);

        Cicada::DrmRequestParam drmRequestParam{};
        drmRequestParam.mDrmType = "WideVine";

        CicadaJSONItem param{};
        param.addValue("requestType", "provision");
        param.addValue("url", std::string(cUrl.getChars()));
        param.addValue("data", CicadaUtils::base64enc(cData, dataLen));
        drmRequestParam.mParam = &param;

        free(cData);

        Cicada::DrmResponseData *drmResponseData = player->mDrmCallback(drmRequestParam);

        if (drmResponseData == nullptr) {
            return nullptr;
        }

        int responseDataSize = 0;
        const char *responseData = drmResponseData->getData(&responseDataSize);

        jbyteArray mResult = nullptr;
        if (responseData != nullptr && responseDataSize > 0) {
            mResult = pEnv->NewByteArray(responseDataSize);
            pEnv->SetByteArrayRegion(mResult, 0, responseDataSize, (jbyte *) (responseData));
        }

        delete drmResponseData;

        return mResult;
    }
    return nullptr;
}

jbyteArray JavaExternalPlayer::java_OnRequestKey(JNIEnv *pEnv, jobject object, jlong nativeInstance,
                                                 jstring licenseUrl, jbyteArray data) {
    auto *player = reinterpret_cast<JavaExternalPlayer *>((long) nativeInstance);
    if (player != nullptr && player->mDrmCallback != nullptr) {
        GetStringUTFChars cUrl(pEnv, licenseUrl);
        char *cData = JniUtils::jByteArrayToChars(pEnv, data);
        int dataLen = pEnv->GetArrayLength(data);

        Cicada::DrmRequestParam drmRequestParam{};
        drmRequestParam.mDrmType = "WideVine";

        CicadaJSONItem param{};
        param.addValue("requestType", "key");
        param.addValue("url", std::string(cUrl.getChars()));
        param.addValue("data", CicadaUtils::base64enc(cData, dataLen));
        drmRequestParam.mParam = &param;

        free(cData);

        Cicada::DrmResponseData *drmResponseData = player->mDrmCallback(drmRequestParam);

        if (drmResponseData == nullptr) {
            return nullptr;
        }

        int responseDataSize = 0;
        const char *responseData = drmResponseData->getData(&responseDataSize);

        jbyteArray mResult = nullptr;
        if (responseData != nullptr && responseDataSize > 0) {
            mResult = pEnv->NewByteArray(responseDataSize);
            pEnv->SetByteArrayRegion(mResult, 0, responseDataSize, (jbyte *) (responseData));
        }

        delete drmResponseData;

        return mResult;
    }
    return nullptr;
}




// ----------------call from c++ ----------


bool JavaExternalPlayer::is_supported(const Cicada::options *opts) {
    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return false;
    }
    jobject jOption = JavaOptions::convertTo(mEnv, const_cast<Cicada::options *>(opts));
    jboolean ret = mEnv->CallStaticBooleanMethod(gj_NativeExternalPlayer_Class,
                                                 gj_NativeExternalPlayer_isSupport, jOption);
    if (jOption != nullptr) {
        mEnv->DeleteLocalRef(jOption);
    }
    return (bool) ret;
}

JavaExternalPlayer::JavaExternalPlayer(const Cicada::options *opts) {
    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    jobject tmpPlayer = mEnv->NewObject(gj_NativeExternalPlayer_Class,
                                        gj_NativeExternalPlayer_init);
    jExternalPlayer = mEnv->NewGlobalRef(tmpPlayer);
    mEnv->DeleteLocalRef(tmpPlayer);

    jobject jOption = JavaOptions::convertTo(mEnv, const_cast<Cicada::options *>(opts));
    mEnv->CallVoidMethod(jExternalPlayer, gj_NativeExternalPlayer_create, (long) this, jOption);
    if (jOption != nullptr) {
        mEnv->DeleteLocalRef(jOption);
    }
}

JavaExternalPlayer::~JavaExternalPlayer() {
    jCallRvPv("Release");
    if (jExternalPlayer == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    mEnv->DeleteGlobalRef(jExternalPlayer);

    if (mStreamCount > 0) {
        for (int i = 0; i < mStreamCount; i++) {
            StreamInfo *info = mStreamInfos[i];
            releaseStreamInfo(info);
            free(info);
        }
        delete[] mStreamInfos;
    }
}


StreamInfo *JavaExternalPlayer::getStreamInfoByIndex(int index) {
    if (mStreamCount > 0) {
        for (int i = 0; i < mStreamCount; i++) {
            StreamInfo *info = mStreamInfos[i];
            if (info->streamIndex == index) {
                return info;
            }
        }
    }
    return nullptr;
}

void JavaExternalPlayer::releaseStreamInfo(StreamInfo *pInfo) {
    if (pInfo->subtitleLang) {
        free(pInfo->subtitleLang);
    }

    if (pInfo->audioLang) {
        free(pInfo->audioLang);
    }

    if (pInfo->description) {
        free(pInfo->description);
    }

}

int JavaExternalPlayer::SetListener(const playerListener &Listener) {
    mPlayerListener = Listener;
    return 0;
}

void JavaExternalPlayer::SetOnRenderCallBack(onRenderFrame cb, void *userData) {
    //TODO
}

void JavaExternalPlayer::SetAudioRenderingCallBack(onRenderFrame cb, void *userData) {
    //TODO
}

void JavaExternalPlayer::SetView(void *view) {
    jobject surface = (jobject) view;
    jCallRvPo("SetView", surface);
}

void JavaExternalPlayer::SetDataSource(const char *url) {
    jCallRvPs("SetDataSource", string(url));
}

void JavaExternalPlayer::Prepare() {
    jCallRvPv("Prepare");
}

void JavaExternalPlayer::Start() {
    jCallRvPv("Start");
}

void JavaExternalPlayer::Pause() {
    jCallRvPv("Pause");
}

StreamType JavaExternalPlayer::SwitchStream(int index) {
    return static_cast<StreamType>(jCallRiPiD("SwitchStream", index, ST_TYPE_UNKNOWN));
}

void JavaExternalPlayer::SeekTo(int64_t seekPos, bool bAccurate) {
    jCallRvPlb("SeekTo", seekPos, bAccurate);
}

int JavaExternalPlayer::Stop() {
    return jCallRiPvD("Stop", -1);
}

PlayerStatus JavaExternalPlayer::GetPlayerStatus() const {
    return (PlayerStatus) jCallRiPvD("GetPlayerStatus", PlayerStatus::PLAYER_IDLE);
}

int64_t JavaExternalPlayer::GetDuration() const {
    return jCallRlPvD("GetDuration", 0);
}

int64_t JavaExternalPlayer::GetPlayingPosition() {
    return jCallRlPvD("GetPlayingPosition", 0);
}

int64_t JavaExternalPlayer::GetBufferPosition() {
    return jCallRlPvD("GetBufferPosition", 0);
}

void JavaExternalPlayer::Mute(bool bMute) {
    jCallRvPlb("Mute", 0, bMute);
}

bool JavaExternalPlayer::IsMute() const {
    return jCallRbPvD("IsMute", false);
}

void JavaExternalPlayer::SetVolume(float volume) {
    jCallRvPf("SetVolume", volume);
}

float JavaExternalPlayer::GetVideoRenderFps() {
    return jCallRfPvD("GetVideoRenderFps", 0.0f);
}

void JavaExternalPlayer::EnterBackGround(bool back) {
    jCallRvPlb("EnterBackGround", 0, back);
}

void JavaExternalPlayer::SetScaleMode(ScaleMode mode) {
    jCallRvPi("SetScaleMode", mode);
}

ScaleMode JavaExternalPlayer::GetScaleMode() {
    return (ScaleMode) jCallRiPvD("GetScaleMode", ScaleMode::SM_FIT);
}

void JavaExternalPlayer::SetRotateMode(RotateMode mode) {
    jCallRvPi("SetRotateMode", mode);
}

RotateMode JavaExternalPlayer::GetRotateMode() {
    return (RotateMode) jCallRiPvD("GetRotateMode", RotateMode::ROTATE_MODE_0);
}

void JavaExternalPlayer::SetMirrorMode(MirrorMode mode) {
    jCallRvPi("SetMirrorMode", mode);
}

MirrorMode JavaExternalPlayer::GetMirrorMode() {
    return (MirrorMode) jCallRiPvD("GetMirrorMode", MirrorMode::MIRROR_MODE_NONE);

}

void JavaExternalPlayer::SetVideoBackgroundColor(uint32_t color) {
    jCallRvPlb("SetVideoBackgroundColor", color, false);
}

int JavaExternalPlayer::GetCurrentStreamIndex(StreamType type) {
    return jCallRiPiD("GetCurrentStreamIndex", type, -1);
}

StreamInfo *JavaExternalPlayer::GetCurrentStreamInfo(StreamType type) {
    int index = GetCurrentStreamIndex(type);
    if (index < 0) {
        return nullptr;
    }

    if (mStreamCount > 0) {
        for (int i = 0; i < mStreamCount; i++) {
            StreamInfo *info = mStreamInfos[i];
            if (info->streamIndex == index) {
                return info;
            }
        }
    }

    return nullptr;
}

int64_t JavaExternalPlayer::GetMasterClockPts() {
    return jCallRlPvD("GetMasterClockPts", 0);
}

void JavaExternalPlayer::SetTimeout(int timeout) {
    jCallRvPi("SetTimeout", timeout);
}

void JavaExternalPlayer::SetDropBufferThreshold(int dropValue) {
    jCallRvPi("SetDropBufferThreshold", dropValue);
}

void JavaExternalPlayer::SetDecoderType(DecoderType type) {
    jCallRvPi("SetDecoderType", type);
}

DecoderType JavaExternalPlayer::GetDecoderType() {
    return (DecoderType) jCallRiPvD("GetDecoderType", DecoderType::DT_SOFTWARE);
}

float JavaExternalPlayer::GetVolume() const {
    return jCallRfPvD("GetVolume", 1.0f);
}

void JavaExternalPlayer::SetRefer(const char *refer) {
    jCallRvPs("SetRefer", string(refer));
}

void JavaExternalPlayer::SetUserAgent(const char *userAgent) {
    jCallRvPs("SetUserAgent", string(userAgent));
}

void JavaExternalPlayer::SetLooping(bool bCirclePlay) {
    jCallRvPlb("SetLooping", 0, bCirclePlay);
}

bool JavaExternalPlayer::isLooping() {
    return jCallRbPvD("isLooping", false);
}

void JavaExternalPlayer::CaptureScreen() {
    jCallRvPv("CaptureScreen");
}

void JavaExternalPlayer::GetVideoResolution(int &width, int &height) {
    width = jCallRiPvD("getVideoWidth", 0);
    height = jCallRiPvD("getVideoHeight", 0);
}

void JavaExternalPlayer::GetVideoRotation(int &rotation) {
    rotation = jCallRiPvD("GetVideoRotation", 0);
}

std::string JavaExternalPlayer::GetPropertyString(PropertyKey key) {
    //TODO
    return ICicadaPlayer::GetPropertyString(key);
}

int64_t JavaExternalPlayer::GetPropertyInt(PropertyKey key) {
    //TODO
    return ICicadaPlayer::GetPropertyInt(key);
}

float JavaExternalPlayer::GetVideoDecodeFps() {
    return jCallRfPvD("GetVideoDecodeFps", 0.0f);
}

int JavaExternalPlayer::SetOption(const char *key, const char *value) {
    //TODO
    return 0;
}

void JavaExternalPlayer::GetOption(const char *key, char *value) {
    //TODO
}

void JavaExternalPlayer::setSpeed(float speed) {
    jCallRvPf("setSpeed", speed);
}

float JavaExternalPlayer::getSpeed() {
    return jCallRfPvD("getSpeed", 1.0f);
}

void JavaExternalPlayer::AddCustomHttpHeader(const char *httpHeader) {
    jCallRvPs("AddCustomHttpHeader", string(httpHeader));
}

void JavaExternalPlayer::RemoveAllCustomHttpHeader() {
    jCallRvPv("RemoveAllCustomHttpHeader");
}

void JavaExternalPlayer::addExtSubtitle(const char *uri) {
    jCallRvPs("addExtSubtitle", string(uri));
}

int JavaExternalPlayer::selectExtSubtitle(int index, bool bSelect) {
    jCallRvPlb("selectExtSubtitle", index, bSelect);
    return 0;
}

int JavaExternalPlayer::getCurrentStreamMeta(Stream_meta *meta, StreamType type) {
    //TODO
    return 0;
}

void JavaExternalPlayer::reLoad() {
    jCallRvPv("reLoad");
}

void JavaExternalPlayer::SetAutoPlay(bool bAutoPlay) {
    jCallRvPlb("SetAutoPlay", 0, bAutoPlay);
}

bool JavaExternalPlayer::IsAutoPlay() {
    return jCallRbPvD("IsAutoPlay", false);
}

int JavaExternalPlayer::invokeComponent(std::string content) {
    //TODO
    return 0;
}


void JavaExternalPlayer::jCallRvPv(const string &name) {
    if (jExternalPlayer == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    NewStringUTF funcName(mEnv, name.c_str());
    mEnv->CallVoidMethod(jExternalPlayer, gj_NativeExternalPlayer_callRvPv,
                         funcName.getString());
}

void JavaExternalPlayer::jCallRvPf(const string &name, float value) {
    if (jExternalPlayer == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    NewStringUTF funcName(mEnv, name.c_str());
    mEnv->CallVoidMethod(jExternalPlayer, gj_NativeExternalPlayer_callRvPf,
                         funcName.getString(), (jfloat) value);
}

void JavaExternalPlayer::jCallRvPi(const string &name, int value) {
    if (jExternalPlayer == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    NewStringUTF funcName(mEnv, name.c_str());
    mEnv->CallVoidMethod(jExternalPlayer, gj_NativeExternalPlayer_callRvPi,
                         funcName.getString(), (jint) value);
}

float JavaExternalPlayer::jCallRfPvD(const string &name, float defaultValue) const {
    if (jExternalPlayer == nullptr) {
        return defaultValue;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return defaultValue;
    }

    NewStringUTF funcName(mEnv, name.c_str());
    jfloat ret = mEnv->CallFloatMethod(jExternalPlayer, gj_NativeExternalPlayer_callRfPvD,
                                       funcName.getString(), (jfloat) defaultValue);
    return (float) ret;
}

int64_t JavaExternalPlayer::jCallRlPvD(const string &name, int64_t defaultValue) const {
    if (jExternalPlayer == nullptr) {
        return defaultValue;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return defaultValue;
    }

    NewStringUTF funcName(mEnv, name.c_str());
    jlong ret = mEnv->CallLongMethod(jExternalPlayer, gj_NativeExternalPlayer_callRlPvD,
                                     funcName.getString(), (jlong) defaultValue);
    return (int64_t) ret;
}

void JavaExternalPlayer::jCallRvPo(const string &name, jobject value) {
    if (jExternalPlayer == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    NewStringUTF funcName(mEnv, name.c_str());
    mEnv->CallVoidMethod(jExternalPlayer, gj_NativeExternalPlayer_callRvPo,
                         funcName.getString(), value);
}

void JavaExternalPlayer::jCallRvPs(const string &name, const string &value) {
    if (jExternalPlayer == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    NewStringUTF funcName(mEnv, name.c_str());
    NewStringUTF valueStr(mEnv, value.c_str());
    mEnv->CallVoidMethod(jExternalPlayer, gj_NativeExternalPlayer_callRvPs,
                         funcName.getString(), valueStr.getString());
}

int JavaExternalPlayer::jCallRiPvD(const string &name, int defaultValue) const {
    if (jExternalPlayer == nullptr) {
        return defaultValue;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return defaultValue;
    }

    NewStringUTF funcName(mEnv, name.c_str());
    jint ret = mEnv->CallIntMethod(jExternalPlayer, gj_NativeExternalPlayer_callRiPvD,
                                   funcName.getString(), (jint) defaultValue);
    return (int) ret;

}

int JavaExternalPlayer::jCallRiPiD(const string &name, int iv, int defaultValue) const {
    if (jExternalPlayer == nullptr) {
        return defaultValue;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return defaultValue;
    }

    NewStringUTF funcName(mEnv, name.c_str());
    jint ret = mEnv->CallIntMethod(jExternalPlayer, gj_NativeExternalPlayer_callRiPiD,
                                   funcName.getString(), (jint) iv, (jint) defaultValue);
    return (int) ret;

}

void JavaExternalPlayer::jCallRvPlb(const string &name, int64_t lv, bool bv) {
    if (jExternalPlayer == nullptr) {
        return;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return;
    }

    NewStringUTF funcName(mEnv, name.c_str());
    mEnv->CallVoidMethod(jExternalPlayer, gj_NativeExternalPlayer_callRvPlb,
                         funcName.getString(), (jlong) lv, (jboolean) bv);
}

bool JavaExternalPlayer::jCallRbPvD(const string &name, bool defaultValue) const {
    if (jExternalPlayer == nullptr) {
        return defaultValue;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return defaultValue;
    }

    NewStringUTF funcName(mEnv, name.c_str());
    jboolean ret = mEnv->CallBooleanMethod(jExternalPlayer, gj_NativeExternalPlayer_callRbPvD,
                                           funcName.getString(), (jboolean) defaultValue);
    return (bool) ret;
}

jobject JavaExternalPlayer::jCallRoPi(const string &name, int value) const {
    if (jExternalPlayer == nullptr) {
        return nullptr;
    }

    JniEnv Jenv;
    JNIEnv *mEnv = Jenv.getEnv();

    if (mEnv == nullptr) {
        return nullptr;
    }

    NewStringUTF funcName(mEnv, name.c_str());
    jobject ret = mEnv->CallObjectMethod(jExternalPlayer, gj_NativeExternalPlayer_callRoPi,
                                         funcName.getString(), (jint) value);
    return ret;
}