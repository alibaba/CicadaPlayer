//
// Created by lifujun on 2020/7/23.
//

#ifndef SOURCE_JAVAEXTERNALPLAYER_H
#define SOURCE_JAVAEXTERNALPLAYER_H


#include <jni.h>
#include <ICicadaPlayer.h>
#include <CicadaPlayerPrototype.h>

class JavaExternalPlayer : public Cicada::ICicadaPlayer, private Cicada::CicadaPlayerPrototype {
public :
    static void init(JNIEnv *pEnv);

    static void unInit(JNIEnv *pEnv);

    static int registerMethod(JNIEnv *pEnv);


    static void java_OnPrepared(JNIEnv *pEnv, jobject object, jlong nativeInstance);

    static void java_OnLoopingStart(JNIEnv *pEnv, jobject object, jlong nativeInstance);

    static void java_OnCompletion(JNIEnv *pEnv, jobject object, jlong nativeInstance);

    static void java_OnFirstFrameShow(JNIEnv *pEnv, jobject object, jlong nativeInstance);

    static void java_OnLoadingStart(JNIEnv *pEnv, jobject object, jlong nativeInstance);

    static void java_OnLoadingEnd(JNIEnv *pEnv, jobject object, jlong nativeInstance);

    static void java_OnAutoPlayStart(JNIEnv *pEnv, jobject object, jlong nativeInstance);

    static void
    java_OnSeeking(JNIEnv *pEnv, jobject object, jlong nativeInstance, jboolean seekInCache);

    static void
    java_OnSeekEnd(JNIEnv *pEnv, jobject object, jlong nativeInstance, jboolean seekInCache);

    static void
    java_OnPositionUpdate(JNIEnv *pEnv, jobject object, jlong nativeInstance, jlong position);

    static void java_OnBufferPositionUpdate(JNIEnv *pEnv, jobject object, jlong nativeInstance,
                                            jlong bufferPosition);

    static void
    java_OnLoadingProgress(JNIEnv *pEnv, jobject object, jlong nativeInstance, jlong progress);

    static void
    java_OnVideoSizeChanged(JNIEnv *pEnv, jobject object, jlong nativeInstance, jint with,
                            jint height);

    static void
    java_OnStatusChanged(JNIEnv *pEnv, jobject object, jlong nativeInstance, jint from, jint to);

    static void
    java_OnVideoRendered(JNIEnv *pEnv, jobject object, jlong nativeInstance, jlong timeMs,
                         jlong pts);

    static void java_OnErrorCallback(JNIEnv *pEnv, jobject object, jlong nativeInstance, jlong code,
                                     jstring msg);

    static void java_OnEventCallback(JNIEnv *pEnv, jobject object, jlong nativeInstance, jlong code,
                                     jstring msg);

    static void
    java_OnStreamInfoGet(JNIEnv *pEnv, jobject object, jlong nativeInstance, jobject mediaInfo);

    static void
    java_OnStreamSwitchSuc(JNIEnv *pEnv, jobject object, jlong nativeInstance, jint type,
                           jobject trackInfo);

    static void java_OnCaptureScreen(JNIEnv *pEnv, jobject object, jlong nativeInstance, jint width,
                                     jint height, jbyteArray datas);

    static void java_OnSubtitleHide(JNIEnv *pEnv, jobject object, jlong nativeInstance, jint index,
                                    jbyteArray content);

    static void java_OnSubtitleShow(JNIEnv *pEnv, jobject object, jlong nativeInstance, jint index,
                                    jbyteArray content);

    static void
    java_OnSubtitleExtAdd(JNIEnv *pEnv, jobject object, jlong nativeInstance, jlong index,
                          jstring url);

    static jbyteArray java_OnRequestProvision(JNIEnv *pEnv, jobject object, jlong nativeInstance, jstring provisionUrl , jbyteArray data);

    static jbyteArray java_OnRequestKey(JNIEnv *pEnv, jobject object, jlong nativeInstance, jstring licenseUrl , jbyteArray data);

public:
    JavaExternalPlayer(const Cicada::options * opts);

    ~JavaExternalPlayer() override;

    string getName() override
    {
        // TODO: get the name form java
        return "ExoPlayer";
    }

    int SetListener(const playerListener &Listener) override;

    void SetOnRenderCallBack(onRenderFrame cb, void *userData) override;

    void SetView(void *view) override;

    void SetDataSource(const char *url) override;


    void Prepare() override;

    void Start() override;

    void Pause() override;


    StreamType SwitchStream(int index) override;

    void SeekTo(int64_t seekPos, bool bAccurate) override;

    int Stop() override;

    PlayerStatus GetPlayerStatus() const override;

    int64_t GetDuration() const override;

    int64_t GetPlayingPosition() override;

    int64_t GetBufferPosition() override;

    void Mute(bool bMute) override;

    bool IsMute() const override;

    void SetVolume(float volume) override;

    float GetVideoRenderFps() override;

    void EnterBackGround(bool back) override;

    void SetScaleMode(ScaleMode mode) override;

    ScaleMode GetScaleMode() override;

    void SetRotateMode(RotateMode mode) override;

    RotateMode GetRotateMode() override;

    void SetMirrorMode(MirrorMode mode) override;

    void SetVideoBackgroundColor(uint32_t color) override;

    MirrorMode GetMirrorMode() override;

    int GetCurrentStreamIndex(StreamType type) override;

    StreamInfo *GetCurrentStreamInfo(StreamType type) override;

    int64_t GetMasterClockPts() override;

    void SetTimeout(int timeout) override;

    void SetDropBufferThreshold(int dropValue) override;

    void SetDecoderType(DecoderType type) override;

    DecoderType GetDecoderType() override;

    float GetVolume() const override;

    void SetRefer(const char *refer) override;

    void SetUserAgent(const char *userAgent) override;

    void SetLooping(bool bCirclePlay) override;

    bool isLooping() override;

    void CaptureScreen() override;

    void GetVideoResolution(int &width, int &height) override;

    void GetVideoRotation(int &rotation) override;

    std::string GetPropertyString(PropertyKey key) override;

    int64_t GetPropertyInt(PropertyKey key) override;

    long GetPropertyLong(int key) override {
        return 0;
    };

    float GetVideoDecodeFps() override;

    int SetOption(const char *key, const char *value) override;

    void GetOption(const char *key, char *value) override;

    void setSpeed(float speed) override;

    float getSpeed() override;

    void AddCustomHttpHeader(const char *httpHeader) override;

    void RemoveAllCustomHttpHeader() override;

    void addExtSubtitle(const char *uri) override;

    int selectExtSubtitle(int index, bool bSelect) override;

    int setStreamDelay(int index, int64_t time) override
    {
        // TODO: tobe impl
        return 0;
    }

    int getCurrentStreamMeta(Stream_meta *meta, StreamType type) override;

    void reLoad() override;

    void SetAutoPlay(bool bAutoPlay) override;

    bool IsAutoPlay() override;

    int invokeComponent(std::string content) override;

    void SetAudioRenderingCallBack(onRenderFrame cb, void *userData) override ;

    void setDrmRequestCallback(const std::function<Cicada::DrmResponseData*(const Cicada::DrmRequestParam& drmRequestParam)> &drmCallback) override{
        mDrmCallback = drmCallback;
    };
public:
    static bool is_supported(const Cicada::options *opts);

private:
    ICicadaPlayer *clone() override {
        return new JavaExternalPlayer(options);
    }

    explicit JavaExternalPlayer(int dummy) {
        addPrototype(this);
    }

    int probeScore(const Cicada::options *opts) override {
        options = opts;
        if (is_supported(opts)) {
            return Cicada::SUPPORT_MAX;
        } else {
            return Cicada::SUPPORT_NOT;
        }
    }

    static JavaExternalPlayer se;


private:
    void jCallRvPv(const string &name);

    void jCallRvPf(const string &name, float value);

    void jCallRvPi(const string &name, int value);

    void jCallRvPlb(const string &name, int64_t lv, bool bv);

    void jCallRvPo(const string &name, jobject value);

    void jCallRvPs(const string &name, const string &value);

    bool jCallRbPvD(const string &name, bool defaultValue) const;

    float jCallRfPvD(const string &name, float defaultValue) const;

    int64_t jCallRlPvD(const string &name, int64_t defaultValue) const;

    int jCallRiPvD(const string &name, int defaultValue) const;

    int jCallRiPiD(const string &name, int iv, int defaultValue) const;

    jobject jCallRoPi(const string &name, int value) const;

private:
    jobject jExternalPlayer = nullptr;


private:
    playerListener mPlayerListener{};
    StreamInfo ** mStreamInfos{nullptr};
    int mStreamCount{0};
    const Cicada::options *options{nullptr};
    std::function<Cicada::DrmResponseData*(const Cicada::DrmRequestParam& drmRequestParam)> mDrmCallback = nullptr;
private:
    void releaseStreamInfo(StreamInfo *pInfo);

    StreamInfo *getStreamInfoByIndex(int index);
};


#endif //SOURCE_JAVAEXTERNALPLAYER_H
