//
// Created by moqi on 2020/7/20.
//

#ifndef CICADAMEDIA_APPLE_AVPLAYER_H
#define CICADAMEDIA_APPLE_AVPLAYER_H

#include "native_cicada_player_def.h"
#import <mutex>

#include "../CicadaPlayerPrototype.h"
namespace Cicada {
    class AppleAVPlayer final : public ICicadaPlayer, private CicadaPlayerPrototype {
    public:
        AppleAVPlayer();
        ~AppleAVPlayer() final;

        string getName() override
        {
            return "AppleAVPlayer";
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

        long GetPropertyLong(int key) override
        {
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

        int setStreamDelay(int index, int64_t time) override;

        int getCurrentStreamMeta(Stream_meta *meta, StreamType type) override;

        void reLoad() override;

        void SetAutoPlay(bool bAutoPlay) override;

        bool IsAutoPlay() override;

        int invokeComponent(std::string content) override;

        void SetAudioRenderingCallBack(onRenderFrame cb, void *userData)
        {}

        void setDrmRequestCallback(const std::function<DrmResponseData*(const DrmRequestParam& drmRequestParam)> &drmCallback) override
        {}

    public:
        static bool is_supported(const options *opts)
        {
            if (opts) {
                string name = opts->get("name");
                if (!name.empty()) {
                    if (name == "AppleAVPlayer") {
                        return true;
                    } else {
                        return false;
                    }
                }
            }
            return false;
        }

    private:
        ICicadaPlayer *clone() override
        {
            return new AppleAVPlayer();
        }
        explicit AppleAVPlayer(int dummy)
        {
            addPrototype(this);
            mIsDummy = true;
        }

        int probeScore(const options *opts) override
        {
            if (is_supported(opts)) {
                return SUPPORT_MAX;
            }
            return SUPPORT_NOT;
        }

        static AppleAVPlayer se;

        void recheckHander();

        void UpdatePlayerStatus(PlayerStatus status);

    private:
        mutable std::recursive_mutex mCreateMutex{};
        void *avPlayer{nullptr};
        void *playerHandler{nullptr};
        playerListener mListener{nullptr};
        float recordVolume = 0.0;
        StreamInfo **mStreamInfos{nullptr};

        void *resourceLoaderDelegate{nullptr};
        void *parentLayer{nullptr};
        void *sourceUrl{nullptr};
        void *subtitleUrl{nullptr};
        bool isAutoPlay = false;
        
        bool mIsDummy{false};
        PlayerStatus mStatus{PLAYER_IDLE};

    };
}// namespace Cicada


#endif//CICADAMEDIA_APPLE_AVPLAYER_H
