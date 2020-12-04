#ifndef CICADA_PLAYER_SERVICE_H
#define CICADA_PLAYER_SERVICE_H

#include <string>

using namespace std;

#include "native_cicada_player_def.h"
#include "demuxer/demuxer_service.h"

#include "MediaPlayerUtil.h"

#include "player_msg_control.h"
#include "buffer_controller.h"

#include <deque>
#include "system_refer_clock.h"

#include "SMPAVDeviceManager.h"
#include "SMP_DCAManager.h"
#include "SuperMediaPlayerDataSourceListener.h"
#include "hls_adaptive_manager.h"
#include "player_notifier.h"
#include "player_types.h"
#include "render/video/IVideoRender.h"
#include <filter/IAudioFilter.h>
#include <queue>
#include <render/audio/IAudioRender.h>
#include <utils/bitStreamParser.h>

#include "CicadaPlayerPrototype.h"
#include <cacheModule/CacheModule.h>
#include <cacheModule/cache/CacheConfig.h>
#include <drm/DrmManager.h>
#include <codec/IDecoder.h>

#ifdef __APPLE__

#include <TargetConditionals.h>

#endif

#include "mediaPlayerSubTitleListener.h"

namespace Cicada {
    using namespace Cicada;
#define HAVE_VIDEO (mCurrentVideoIndex >= 0)
#define HAVE_AUDIO (mCurrentAudioIndex >= 0)
#define HAVE_SUBTITLE (mCurrentSubtitleIndex >= 0)

    typedef struct streamTime_t {
        int64_t startTime;
        int64_t deltaTime;
        int64_t deltaTimeTmp;

    } streamTime;

    typedef enum RENDER_RESULT {
        RENDER_NONE,
        RENDER_PARTLY,
        RENDER_FULL,
    } RENDER_RESULT;

    typedef enum APP_STATUS {
        APP_FOREGROUND,
        APP_BACKGROUND,
    } APP_STATUS;


    class SuperMediaPlayer : public ICicadaPlayer, private PlayerMessageControllerListener, private CicadaPlayerPrototype {

        friend class SuperMediaPlayerDataSourceListener;
        friend class SMP_DCAManager;

    public:

        SuperMediaPlayer();

        ~SuperMediaPlayer() override;

        string getName() override
        {
            return "SuperMediaPlayer";
        }

        int SetListener(const playerListener &Listener) override;

        void SetOnRenderCallBack(onRenderFrame cb, void *userData) override;

        void SetAudioRenderingCallBack(onRenderFrame cb, void *userData) override;

        // TODO: use setParameters and setOpt to set
        void SetRefer(const char *refer) override;

        void SetUserAgent(const char *userAgent) override;

        void SetTimeout(int timeout) override;

        void SetDropBufferThreshold(int dropValue) override;

        void SetLooping(bool looping) override;

        bool isLooping() override;

        int SetOption(const char *key, const char *value) override;

        void GetOption(const char *key, char *value) override;

        int64_t GetPlayingPosition() override
        {
            return getCurrentPosition() / 1000;
        };

        int64_t GetBufferPosition() override;

        int64_t GetDuration() const override;

        PlayerStatus GetPlayerStatus() const override;

        void SetScaleMode(ScaleMode mode) override;

        ScaleMode GetScaleMode() override;

        void SetRotateMode(RotateMode mode) override;

        RotateMode GetRotateMode() override;

        void SetMirrorMode(MirrorMode mode) override;

        void SetVideoBackgroundColor(uint32_t color) override;

        MirrorMode GetMirrorMode() override;

        int GetCurrentStreamIndex(StreamType type) override;

        StreamInfo *GetCurrentStreamInfo(StreamType type) override;

        float GetVolume() const override;

        void CaptureScreen() override;

        void SetDecoderType(DecoderType type) override;

        DecoderType GetDecoderType() override;

        bool IsMute() const override;

        float GetVideoRenderFps() override;

        float GetVideoDecodeFps() override;

        void GetVideoResolution(int &width, int &height) override;

        void GetVideoRotation(int &rotation) override;

        void SetView(void *view) override;

        void SetDataSource(const char *url) override;

        void Prepare() override;

        void SetVolume(float volume) override;

        void Start() override;

        void Pause() override;

        void SeekTo(int64_t pos, bool bAccurate) override;

        void Mute(bool bMute) override;

        void EnterBackGround(bool back) override;

        StreamType SwitchStream(int streamIndex) override;

        int Stop() final;

        void setSpeed(float speed) override;

        void AddCustomHttpHeader(const char *httpHeader) override;

        void RemoveAllCustomHttpHeader() override;

        float getSpeed() override;

        void Interrupt(bool inter);

        std::string GetPropertyString(PropertyKey key) override;

        int64_t GetPropertyInt(PropertyKey key) override;

        int64_t GetMasterClockPts() override;

        int getCurrentStreamMeta(Stream_meta *meta, StreamType type) override;

        void reLoad() override;

        void SetAutoPlay(bool bAutoPlay) override;

        bool IsAutoPlay() override;

        void addExtSubtitle(const char *uri) override;

        int selectExtSubtitle(int index, bool bSelect) override;

        int invokeComponent(std::string content) override;

        void setDrmRequestCallback(const std::function<DrmResponseData*(const DrmRequestParam& drmRequestParam)>  &drmCallback) override;

    private:
        void NotifyPosition(int64_t position);

        void OnTimer(int64_t curTime);

        int updateLoopGap();

        int mainService();

        bool NeedDrop(int64_t pts, int64_t refer);

        void NotifyError(int code);

        void putMsg(PlayMsgType type, const MsgParam &param, bool trigger = true);

        void ProcessVideoLoop();

        void OnDemuxerCallback(const std::string &key, const std::string &value);

        bool DoCheckBufferPass();

        int DecodeVideoPacket(unique_ptr<IAFPacket> &pVideoPacket);

        void LiveCatchUp(int64_t delayTime);

        int FillVideoFrame();

    private:
        int SetUpAudioPath();

        int setUpAudioDecoder(const Stream_meta *meta);

        int SetUpVideoPath();

        void SendVideoFrameToRender(unique_ptr<IAFFrame> frame, bool valid = true);

        // RENDER_NONE: doesn't render
        // RENDER_PARTLY: render part of data
        // RENDER_FULL: render a frame fully
        RENDER_RESULT RenderAudio();

        int DecodeAudio(unique_ptr<IAFPacket> &pPacket);

        int ReadPacket();

        void PostBufferPositionMsg();

        void ChangePlayerStatus(PlayerStatus newStatus);

        void ResetSeekStatus();

        static void VideoRenderCallback(void *arg, int64_t pts, void *userData);

        void Reset();

        void FlushSubtitleInfo();

        void FlushAudioPath();

        void FlushVideoPath();

        bool SeekInCache(int64_t pos);

        void SwitchVideo(int64_t startTime);

        int64_t getPlayerBufferDuration(bool gotMax, bool internal);

        void ProcessOpenStreamInit(int streamIndex);

        static int64_t getAudioPlayTimeStampCB(void *arg);

        int64_t getAudioPlayTimeStamp();

        bool render();

        void RenderSubtitle(int64_t pts);

        bool RenderVideo(bool force_render);

        void releaseStreamInfo(const StreamInfo *info) const;

        int openUrl();

        // mSeekFlag will be set when processing (after remove from mMessageControl), it have gap
        bool isSeeking()
        {
            return INT64_MIN != mSeekPos;
        } //{return mSeekFlag || mMessageControl.findMsgByType(MSG_SEEKTO);}

//        void setRotationMode(RotateMode rotateMode, MirrorMode mirrorMode) const;

        bool CreateVideoRender(uint64_t flags);

        int CreateVideoDecoder(bool bHW, Stream_meta &meta);

        int64_t getCurrentPosition();

        void switchSubTitle(int index);

        void switchAudio(int index);

        void switchVideoStream(int index, Stream_type type);

        void checkEOS();

        void notifySeekEndCallback();

        void notifyPreparedCallback();

        void updateVideoMeta();

        class ApsaraAudioRenderCallback : public IAudioRenderListener {
        public:
            explicit ApsaraAudioRenderCallback(SuperMediaPlayer &player) : mPlayer(player)
            {}

            void onEOS() override
            {}

            bool onInterrupt(bool interrupt) override
            {
                if (interrupt) {
                    if (mPlayer.mPlayStatus == PLAYER_PLAYING) {
                        mPlayer.Pause();
                        mPlayer.mPausedByAudioInterrupted = true;
                    }
                } else {
                    if (mPlayer.mPlayStatus == PLAYER_PAUSED && mPlayer.mPausedByAudioInterrupted) {
                        mPlayer.Start();
                    }
                }

                return true;
            }

        private:
            SuperMediaPlayer &mPlayer;
        };

    private:
        void checkFirstRender();

        bool OnPlayerMsgIsPadding(PlayMsgType msg, MsgParam msgContent) final;

        void ProcessPrepareMsg() final;

        void ProcessStartMsg() final;

        void ProcessSetDisplayMode() final;

        void ProcessSetRotationMode() final;

        void ProcessSetMirrorMode() final;

        void ProcessSetVideoBackgroundColor() final;

        void ProcessSetViewMsg(void *view) final;

        void ProcessSetDataSourceMsg(const std::string &url) final;

        void ProcessPauseMsg() final;

        void ProcessSeekToMsg(int64_t seekPos, bool bAccurate) final;

        void ProcessMuteMsg() final;

        void ProcessSwitchStreamMsg(int index) final;

        void ProcessVideoRenderedMsg(int64_t pts, int64_t timeMs, void *picUserData) final;

        void ProcessVideoCleanFrameMsg() final;

        void ProcessVideoHoldMsg(bool hold) final;

        void ProcessAddExtSubtitleMsg(const std::string &url) final;

        void ProcessSelectExtSubtitleMsg(int index, bool select) final;

        void ProcessSetSpeed(float speed) final;


    private:
        static IVideoRender::Scale convertScaleMode(ScaleMode mode);

        static IVideoRender::Rotate convertRotateMode(RotateMode mode);

        static IVideoRender::Flip convertMirrorMode(MirrorMode mode);


    public:
        static bool is_supported(const options *opts)
        {
            return true;
        }

    private:
        explicit SuperMediaPlayer(int dummy)
        {
            mIsDummy = true;
            addPrototype(this);
        }
        ICicadaPlayer *clone() override
        {
            return new SuperMediaPlayer();
        };

        static SuperMediaPlayer se;


    private:
        IDataSource *mDataSource{nullptr};
        std::atomic_bool mCanceled{false};
        demuxer_service *mDemuxerService{nullptr};
        std::unique_ptr<DrmManager> mDrmManager{};
        std::queue<unique_ptr<IAFFrame>> mVideoFrameQue{};
        std::deque<unique_ptr<IAFFrame>> mAudioFrameQue{};
        unique_ptr<streamMeta> mCurrentVideoMeta{};
        bool videoDecoderEOS = false;
        bool audioDecoderEOS = false;
        picture_cache_type mPictureCacheType = picture_cache_type_cannot;
        bool videoDecoderFull = false;
        std::unique_ptr<PlayerMessageControl> mMessageControl{nullptr};
        std::unique_ptr<ApsaraAudioRenderCallback> mAudioRenderCB{nullptr};
        std::unique_ptr<BufferController> mBufferController{nullptr};

        std::mutex mAppStatusMutex;
        std::atomic<APP_STATUS> mAppStatus{APP_FOREGROUND};
//#ifdef WIN32
//        AlivcDxva2Render m_dxva2Render;
//#endif
        int mVideoWidth{0};
        int mVideoHeight{0};
        int mVideoRotation{0};
        int64_t mDuration{INT64_MIN};
        int64_t mBufferPosition{0};
        PlayerStatus mOldPlayStatus{PLAYER_IDLE};
        atomic <PlayerStatus> mPlayStatus{PLAYER_IDLE};
        std::deque<std::unique_ptr<IAFPacket>> mSubtitleShowedQueue;
        std::deque<StreamInfo *> mStreamInfoQueue;
        StreamInfo **mStreamInfos{nullptr};
//        ResolutionPolicy mResolutionPolicy{kShowAll};
        int mCurrentVideoIndex{-1};
        int mCurrentAudioIndex{-1};
        int mCurrentSubtitleIndex{-1};
        int mWillChangedVideoStreamIndex{-1};
        int mWillChangedAudioStreamIndex{-1};
        int mWillChangedSubtitleStreamIndex{-1};
        int mRemainLiveSegment{0};// To avoid access demuxer multi-thread
        bool mInited{false};
        atomic_bool mSeekNeedCatch{false};
        const static int64_t SEEK_ACCURATE_MAX;
        atomic <int64_t> mSeekPos{INT64_MIN};
        SystemReferClock mMasterClock;
        streamTime mAudioTime{INT64_MIN, 0};
        int64_t mPlayedVideoPts{INT64_MIN}; // sync pts
        bool mVideoPtsRevert{false};
        bool mAudioPtsRevert{false};
        int64_t mPlayedAudioPts{INT64_MIN};
        int64_t mFirstVideoPts{INT64_MIN};
        int64_t mCurVideoPts{INT64_MIN};  // update from render cb
        int64_t mFirstAudioPts{INT64_MIN};
        int64_t mMediaStartPts{INT64_MIN}; // the first small frame pts in the media stream
        int64_t mVideoChangedFirstPts{INT64_MIN};
        int64_t mAudioChangedFirstPts{INT64_MIN};
        int64_t mSubtitleChangedFirstPts{INT64_MIN};
        int64_t mFirstReadPacketSucMS{0};
        int mMainStreamId{-1};
        int64_t mRemovedFirstAudioPts{INT64_MIN};;
        int64_t mFirstSeekStartTime{0};
        bool mEof{false};
        bool mSubtitleEOS{false};
        bool mLowMem{false};
        bool mSeekFlag{false};
        bool mSeekInCache{false};
        bool mFirstBufferFlag{true}; // first play and after seek play
        bool mBufferingFlag{false};
        bool mMixMode{false};
        bool mAdaptiveVideo{false};
        bool mFirstRendered{false};
        int mWriteAudioLen{0};
        int64_t mLastAudioFrameDuration{INT64_MIN};
        int64_t mTimeoutStartTime{INT64_MIN};
        int64_t mSubtitleShowIndex{0};
        bool mBufferIsFull{false};
        bool mWillSwitchVideo{false};
        std::unique_ptr<player_type_set> mSet{};
        int64_t mSoughtVideoPos{INT64_MIN};
        int mTimerInterval = 0;
        int64_t mTimerLatestTime = 0;
        std::mutex mCreateMutex{}; // need lock if access pointer outside of loop thread
        std::mutex mPlayerMutex{};
        std::mutex mSleepMutex{};
        std::condition_variable mPlayerCondition;
        PlayerNotifier *mPNotifier = nullptr;
        std::unique_ptr<afThread> mApsaraThread{};
        int mLoadingProcess{0};
        int64_t mPrepareStartTime = 0;

        int mVideoParserTimes = 0;
        InterlacedType mVideoInterlaced = InterlacedType_UNKNOWN;
        bitStreamParser *mVideoParser = nullptr;

        std::unique_ptr<MediaPlayerUtil> mUtil{};

        std::unique_ptr<SuperMediaPlayerDataSourceListener> mSourceListener{nullptr};
        std::unique_ptr<SMP_DCAManager> mDcaManager{nullptr};

        std::unique_ptr<SMPAVDeviceManager> mAVDeviceManager{nullptr};

        std::unique_ptr<IAFPacket> mVideoPacket{};
        std::unique_ptr<IAFPacket> mAudioPacket{};
        std::unique_ptr<mediaPlayerSubTitleListener> mSubListener;
        std::unique_ptr<subTitlePlayer> mSubPlayer;

        bool dropLateVideoFrames = false;
        bool waitingForStart = false;
        bool mBRendingStart {false};
        bool mSecretPlayBack{false};
        bool mDrmKeyValid{false};

    private:

        bool mAutoPlay = false;

        void doDeCode();

        void doRender();

        void doReadPacket();

        int setUpAudioRender(const IAFFrame::audioInfo &info);

        int64_t mCurrentPos = 0;

        void printTimePosition(int64_t time) const;

        void setUpAVPath();

        void startRendering(bool start);

        void sendDCAMessage();

        int64_t mCheckAudioQueEOSTime{INT64_MIN};
        uint64_t mAudioQueDuration{UINT64_MAX};

        onRenderFrame mFrameCb{nullptr};
        void *mFrameCbUserData{nullptr};

        onRenderFrame mAudioRenderingCb{nullptr};
        void *mAudioRenderingCbUserData{nullptr};
        bool mIsDummy{false};
        bool mPausedByAudioInterrupted{false};
        bool mNeedVideoRender{true};
    };
}// namespace Cicada
#endif// CICADA_PLAYER_SERVICE_H
