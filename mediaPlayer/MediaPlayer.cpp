//
//  MediaPlayer.cpp
//
//  Created by shiping.csp on 2018/11/12.
//

#include <muxer/ffmpegMuxer/FfmpegMuxer.h>
#include <utils/frame_work_log.h>
#include <utils/file/FileUtils.h>
#include <utils/af_string.h>
#include "MediaPlayer.h"
#include "media_player_api.h"
#include "abr/AbrManager.h"
#include "abr/AbrBufferAlgoStrategy.h"
#include "abr/AbrBufferRefererData.h"

#include "analytics/AnalyticsCollectorFactory.h"
#include "analytics/AnalyticsQueryListener.h"
#include "media_player_error_def.h"
#include "PlayerCacheDataSource.h"

using namespace Cicada;

namespace Cicada {

#define GET_PLAYER_HANDLE  playerHandle* handle = (playerHandle*)mPlayerHandle;
#define GET_MEDIA_PLAYER MediaPlayer* player = (MediaPlayer*)userData;

    class QueryListener : public AnalyticsQueryListener {
    public:
        explicit QueryListener(MediaPlayer *player)
        {
            mPlayer = player;
        }

        ~QueryListener() override = default;;

        // analytics query interface
        int64_t OnAnalyticsGetCurrentPosition() override
        {
            if (mPlayer) {
                return mPlayer->GetCurrentPosition();
            }

            return -1;
        }

        int64_t OnAnalyticsGetBufferedPosition() override
        {
            if (mPlayer) {
                return mPlayer->GetBufferedPosition();
            }

            return -1;
        }

        int64_t OnAnalyticsGetDuration() override
        {
            if (mPlayer) {
                return mPlayer->GetDuration();
            }

            return -1;
        }

        std::string OnAnalyticsGetPropertyString(PropertyKey key) override
        {
            if (mPlayer) {
                return mPlayer->GetPropertyString(key);
            }

            return "";
        }

    private:
        MediaPlayer *mPlayer = nullptr;
    };

    MediaPlayer::MediaPlayer()
            : MediaPlayer(*(AnalyticsCollectorFactory::Instance()))
    {
    }

    MediaPlayer::MediaPlayer(IAnalyticsCollectorFactory &factory)
            : mCollectorFactory(factory)
    {
        playerHandle *handle = CicadaCreatePlayer();
        mPlayerHandle = (void *) handle;
        playerListener listener{nullptr};
        listener.userData = this;
        listener.EventCallback = eventCallback;
        listener.ErrorCallback = errorFrameCallback;
        listener.LoopingStart = loopingStartCallback;
        listener.Prepared = preparedCallback;
        listener.Completion = completionCallback;
        listener.FirstFrameShow = firstFrameCallback;
        listener.VideoSizeChanged = videoSizeChangedCallback;
        listener.VideoRendered = videoRenderedCallback;
        listener.PositionUpdate = currentPostionCallback;
        listener.BufferPositionUpdate = bufferPostionCallback;
        listener.LoadingStart = loadingStartCallback;
        listener.LoadingEnd = loadingEndCallback;
        listener.LoadingProgress = loadingProgressCallback;
        listener.Seeking = PlayerSeeking;
        listener.SeekEnd = PlayerSeekEnd;
        listener.SubtitleShow = subtitleShowCallback;
        listener.SubtitleHide = subtitleHideCallback;
        listener.SubtitleExtAdd = subtitleExtAddedCallback;
        listener.StreamInfoGet = streamInfoGetCallback;
        listener.StreamSwitchSuc = streamChangedSucCallback;
        listener.StatusChanged = PlayerStatusChanged;
        listener.CaptureScreen = captureScreenResult;
        listener.AutoPlayStart = autoPlayStart;
        CicadaSetListener(handle, listener);
        CicadaSetMediaFrameCb(handle, onMediaFrameCallback, this);
        mConfig = new MediaPlayerConfig();
        configPlayer(mConfig);
        mQueryListener = new QueryListener(this);
        mCollector = mCollectorFactory.createAnalyticsCollector(mQueryListener);
        mAbrManager = new AbrManager();
        std::function<void(int)> fun = [this](int stream) -> void {
            return this->abrChanged(stream);
        };
        mAbrAlgo = new AbrBufferAlgoStrategy(fun);
        AbrBufferRefererData *pRefererData = new AbrBufferRefererData(handle);
        mAbrAlgo->SetRefererData(pRefererData);
        mAbrManager->SetAbrAlgoStrategy(mAbrAlgo);
    }

    void MediaPlayer::dummyFunction(bool dummy)
    {
#ifdef ENABLE_MUXER
        if (dummy) {
            FfmpegMuxer ffmpegMuxer("", "");
        }
#endif
    }

    MediaPlayer::~MediaPlayer()
    {
        delete mQueryListener;
        delete mAbrManager;
#ifdef ENABLE_CACHE_MODULE
        if (mCacheManager != nullptr) {
            delete mCacheManager;
            mCacheManager = nullptr;
        }
#endif
        delete mAbrAlgo;
        playerHandle *handle = (playerHandle *) mPlayerHandle;
        delete mConfig;
        CicadaReleasePlayer(&handle);

        if (mCollector) {
            mCollectorFactory.destroyAnalyticsCollector(mCollector);
            // avoid be used in derivative class
            mCollector = nullptr;
        }
    }

    int64_t MediaPlayer::GetMasterClockPts()
    {
        GET_PLAYER_HANDLE
        return CicadaGetMasterClockPts(handle);
    }
    void MediaPlayer::SetClockRefer(clockRefer cb, void *arg)
    {
        GET_PLAYER_HANDLE
        return CicadaSetClockRefer(handle,cb,arg);
    }

    void MediaPlayer::abrChanged(int stream)
    {
        std::lock_guard<std::mutex> lock(mMutexAbr);

        // return the selection if disable by selectTrack
        if (!mAbrManager->IsEnableAbr()) {
            return;
        }

        GET_PLAYER_HANDLE
        CicadaSwitchStreamIndex(handle, stream);
    }

    void MediaPlayer::SetListener(const playerListener &Listener)
    {
        mListener = Listener;
    }

    void MediaPlayer::EnableHardwareDecoder(bool bEnabled)
    {
        GET_PLAYER_HANDLE
        DecoderType type = DT_SOFTWARE;

        if (bEnabled) {
            type = DT_HARDWARE;
        }

        CicadaSetDecoderType(handle, type);
    }

    void MediaPlayer::SetView(void *view)
    {
        GET_PLAYER_HANDLE
        CicadaSetView(handle, view);
    }

    void MediaPlayer::SetDataSource(const char *url)
    {
        GET_PLAYER_HANDLE
        string playUrl;
#ifdef ENABLE_CACHE_MODULE
        if (mCacheConfig.mEnable) {
            if (mCacheManager != nullptr) {
                delete mCacheManager;
                mCacheManager = nullptr;
            }

            mCacheManager = new CacheManager();
            mCacheManager->setCacheConfig(mCacheConfig);
            mCacheManager->setSourceUrl(url);
            char descriptionLen[MAX_OPT_VALUE_LENGTH] = {0};
            CicadaGetOption(handle, "descriptionLen", descriptionLen);
            int len = atoi(descriptionLen);
            char *value = static_cast<char *>(malloc(len + 1));
            memset(value, 0, len + 1);
            CicadaGetOption(handle, "description", value);
            mCacheManager->setDescription(value);
            free(value);
            mCacheManager->setCacheFailCallback([this](int code, string msg) -> void {
                AF_LOGE("Cache fail : code = %d , msg = %s", code, msg.c_str());
                this->eventCallback(MEDIA_PLAYER_EVENT_CACHE_ERROR, msg.c_str(), this);
            });
            mCacheManager->setCacheSuccessCallback([this]() -> void {
                if (IsLoop()) {
                    //if cache success and want play loop,
                    // we set loop false to let onCompletion callback deal loop.
                    CicadaSetLoop(static_cast<playerHandle *>(mPlayerHandle), false);
                }

                this->eventCallback(MEDIA_PLAYER_EVENT_CACHE_SUCCESS, nullptr, this);
            });
            ICacheDataSource *cacheDataSource = new PlayerCacheDataSource(mPlayerHandle);
            mCacheManager->setDataSource(cacheDataSource);
            playUrl = mCacheManager->init();
        }else
#endif
        {
            playUrl = url;
        }

        if (playUrl != string(url) && mPlayUrlChangedCallback != nullptr) {
            mPlayUrlChangedCallback(playUrl);
        }

        mPlayUrl = playUrl;
        CicadaSetDataSourceWithUrl(handle, playUrl.c_str());
    }

    void MediaPlayer::SelectTrack(int index)
    {
        std::lock_guard<std::mutex> lock(mMutexAbr);
        GET_PLAYER_HANDLE

        if (SELECT_TRACK_VIDEO_AUTO == index) {
            mAbrManager->EnableAbr(true);
            return;
        } else if (index < SELECT_TRACK_VIDEO_AUTO) {
            return;
        }

        StreamType type = CicadaSwitchStreamIndex(handle, index);

        if (ST_TYPE_VIDEO == type) {
            mAbrManager->EnableAbr(false);
        }
    }

    void MediaPlayer::Prepare()
    {
        if (mCollector) {
            mCollector->ReportBlackInfo();
            mCollector->ReportPrepare();
        }

        GET_PLAYER_HANDLE
        CicadaPreparePlayer(handle);
    }

    void MediaPlayer::Start()
    {
        if (mCollector) {
            mCollector->ReportPlay();
        }

        mAbrManager->Start();
        GET_PLAYER_HANDLE
        CicadaStartPlayer(handle);
    }

    void MediaPlayer::Pause()
    {
        if (mCollector) {
            mCollector->ReportPause();
        }

        mAbrManager->Pause();
        GET_PLAYER_HANDLE
        CicadaPausePlayer(handle);
    }

    void MediaPlayer::SetVolume(float volume)
    {
        GET_PLAYER_HANDLE
        CicadaSetVolume(handle, volume);
    }

    float MediaPlayer::GetVolume()
    {
        GET_PLAYER_HANDLE
        return CicadaGetVolume(handle);
    }

    void MediaPlayer::SeekTo(int64_t seekPos, SeekMode mode)
    {
        if (mCollector) {
            mCollector->ReportSeekStart(seekPos, mode);
        }

        //TODO: use mode to seek
        GET_PLAYER_HANDLE
        bool bAccurate = false;

        if (mode & SEEK_MODE_ACCURATE) {
            bAccurate = true;
        }

        CicadaSeekToTime(handle, seekPos, bAccurate);
        //when seek, close abrmanager
        mAbrManager->Pause();
    }

    void MediaPlayer::CaptureScreen()
    {
        GET_PLAYER_HANDLE
        CicadaCaptureScreen(handle);

        if (mCollector) {
            mCollector->ReportSnapshot();
        }
    }

    void MediaPlayer::Stop()
    {
        if (mCollector) {
            mCollector->ReportStop();
        }

        mAbrManager->Stop();
        mAbrManager->Reset();
        mAbrManager->EnableAbr(false);
        mAbrAlgo->Clear();
#ifdef ENABLE_CACHE_MODULE
        if (mCacheManager != nullptr) {
            mCacheManager->stop("cache stopped by stop");
        }
#endif
        waitingForLoop = false;
        waitingForStart = false;
        GET_PLAYER_HANDLE
        CicadaStopPlayer(handle);
    }

    int64_t MediaPlayer::GetDuration()
    {
        GET_PLAYER_HANDLE
        return CicadaGetDuration(handle);
    }

    int MediaPlayer::GetCurrentStreamIndex(StreamType type)
    {
        GET_PLAYER_HANDLE
        return CicadaGetCurrentStreamIndex(handle, type);
    }

    StreamInfo *MediaPlayer::GetCurrentStreamInfo(StreamType type)
    {
        GET_PLAYER_HANDLE
        return CicadaGetCurrentStreamInfo(handle, type);
    }

    int64_t MediaPlayer::GetCurrentPosition()
    {
        GET_PLAYER_HANDLE
        return CicadaGetCurrentPosition(handle);
    }

    int64_t MediaPlayer::GetBufferedPosition()
    {
        GET_PLAYER_HANDLE
        return CicadaGetCurrentBufferedPosition(handle);
    }

    void MediaPlayer::EnterBackGround(bool back)
    {
        GET_PLAYER_HANDLE
        CicadaEnterBackGround(handle, back);
    }

    void MediaPlayer::SetMute(bool bMute)
    {
        GET_PLAYER_HANDLE
        CicadaSetMute(handle, bMute);
    }

    bool MediaPlayer::IsMuted()
    {
        GET_PLAYER_HANDLE
        return CicadaIsMute(handle);
    }

    void MediaPlayer::SetConfig(const MediaPlayerConfig *config)
    {
        //TODO:SetConfig
        *mConfig = *config;
        configPlayer(mConfig);
    }

    void MediaPlayer::configPlayer(const MediaPlayerConfig *config) const
    {
        GET_PLAYER_HANDLE
        //100
        MediaPlayerConfig playerConfig = *config;

        if (playerConfig.maxDelayTime < 0) {
            playerConfig.maxDelayTime = 0;
        }

        if (playerConfig.networkTimeout < 0) {
            playerConfig.networkTimeout = 0;
        }

        if (playerConfig.startBufferDuration < 0) {
            playerConfig.startBufferDuration = 0;
        }

        if (playerConfig.highBufferDuration < 0) {
            playerConfig.highBufferDuration = 0;
        }

        if (playerConfig.maxDelayTime > playerConfig.maxBufferDuration) {
            playerConfig.maxDelayTime = playerConfig.maxBufferDuration;
        }

        //must
        if ((0 < playerConfig.maxDelayTime)
                && (playerConfig.maxDelayTime < playerConfig.highBufferDuration)) {
            playerConfig.highBufferDuration = playerConfig.maxDelayTime;
        }

        if (playerConfig.startBufferDuration > playerConfig.maxBufferDuration) {
            playerConfig.startBufferDuration = playerConfig.maxBufferDuration;
        }

        //must
        if (playerConfig.highBufferDuration > playerConfig.maxBufferDuration) {
            playerConfig.maxBufferDuration = playerConfig.highBufferDuration;
        }

        CicadaSetDropBufferThreshold(handle, playerConfig.maxDelayTime);
        CicadaSetRefer(handle, playerConfig.referer.c_str());
        CicadaSetTimeout(handle, playerConfig.networkTimeout);
        CicadaSetUserAgent(handle, playerConfig.userAgent.c_str());
        string chStartBufferDur = to_string(playerConfig.startBufferDuration);
        CicadaSetOption(handle, "startBufferDuration", chStartBufferDur.c_str());
        string chMaxDelayTime = to_string(playerConfig.maxDelayTime);
        CicadaSetOption(handle, "RTMaxDelayTime", chMaxDelayTime.c_str());
        string chHighBufDur = to_string(playerConfig.highBufferDuration);
        CicadaSetOption(handle, "highLevelBufferDuration", chHighBufDur.c_str());
        string chMaxBufDur = to_string(playerConfig.maxBufferDuration);
        CicadaSetOption(handle, "maxBufferDuration", chMaxBufDur.c_str());
        CicadaSetOption(handle, "http_proxy", playerConfig.httpProxy.c_str());
        CicadaSetOption(handle, "ClearShowWhenStop", playerConfig.bClearShowWhenStop ? "1" : "0");
        CicadaSetOption(handle, "enableVideoTunnelRender", playerConfig.bEnableTunnelRender ? "1" : "0");
        CicadaRemoveAllCustomHttpHeader(handle);

        //add custom http header
        for (int i = 0; i < playerConfig.customHeaders.size(); i++) {
            CicadaAddCustomHttpHeader(handle, playerConfig.customHeaders[i].c_str());
        }

        *mConfig = playerConfig;
    }

    const MediaPlayerConfig *MediaPlayer::GetConfig()
    {
        return mConfig;
    }

    void MediaPlayer::SetScaleMode(ScaleMode mode)
    {
        GET_PLAYER_HANDLE
        CicadaSetScaleMode(handle, mode);
    }

    ScaleMode MediaPlayer::GetScaleMode()
    {
        GET_PLAYER_HANDLE
        return CicadaGetScaleMode(handle);
    }

    void MediaPlayer::SetLoop(bool bLoop)
    {
        mLoop = bLoop;

        if (mCollector) {
            mCollector->ReportLooping(bLoop);
        }

        GET_PLAYER_HANDLE
        CicadaSetLoop(handle, bLoop);
#ifdef ENABLE_CACHE_MODULE
        //if cache successed before setLoop.
        if (mCacheManager != nullptr) {
            CacheModule::CacheStatus cacheStatus = mCacheManager->getCacheStatus();

            if (cacheStatus == CacheModule::CacheStatus::success && IsLoop()) {
                //if cache success and want play loop,
                // we set loop false to let onCompletion callback deal loop.
                CicadaSetLoop(handle, false);
            }
        }
#endif
    }

    bool MediaPlayer::IsLoop()
    {
        return mLoop;//CicadaGetLoop(handle);
    }

    int MediaPlayer::GetVideoWidth()
    {
        int width = 0;
        int height = 0;
        GET_PLAYER_HANDLE
        CicadaGetVideoResolution(handle, width, height);
        return width;
    }

    int MediaPlayer::GetVideoHeight()
    {
        int width = 0;
        int height = 0;
        GET_PLAYER_HANDLE
        CicadaGetVideoResolution(handle, width, height);
        return height;
    }

    int MediaPlayer::GetVideoRotation()
    {
        int rotation = 0;
        GET_PLAYER_HANDLE
        CicadaGetVideoRotation(handle, rotation);
        return rotation;
    }


    void MediaPlayer::SetRotateMode(RotateMode mode)
    {
        if (mCollector) {
            mCollector->ReportRotate(mode);
        }

        GET_PLAYER_HANDLE
        CicadaSetRotateMode(handle, mode);
    }

    RotateMode MediaPlayer::GetRotateMode()
    {
        GET_PLAYER_HANDLE
        return CicadaGetRotateMode(handle);
    }

    void MediaPlayer::SetMirrorMode(MirrorMode mode)
    {
        if (mCollector) {
            mCollector->ReportRenderMirrorMode(mode);
        }

        GET_PLAYER_HANDLE
        CicadaSetMirrorMode(handle, mode);
    }

    MirrorMode MediaPlayer::GetMirrorMode()
    {
        GET_PLAYER_HANDLE
        return CicadaGetMirrorMode(handle);
    }

    void MediaPlayer::SetVideoBackgroundColor(uint32_t color)
    {
        GET_PLAYER_HANDLE
        CicadaSetVideoBackgroundColor(handle, color);
    }

    void MediaPlayer::SetSpeed(float speed)
    {
        GET_PLAYER_HANDLE
        CicadaPlayerSetSpeed(handle, speed);
    }

    float MediaPlayer::GetSpeed()
    {
        GET_PLAYER_HANDLE
        return CicadaPlayerGetSpeed(handle);
    }

    void MediaPlayer::SetTraceID(const char *traceID)
    {
        //TODO:
    }

    void MediaPlayer::SetComponentCb(player_component_type type, void *factory)
    {
        GET_PLAYER_HANDLE
        CicadaSetComponentCb(handle, type, factory);
    }

    std::string MediaPlayer::GetPropertyString(PropertyKey key)
    {
        GET_PLAYER_HANDLE
        return CicadaGetPropertyString(handle, key);
    }

    void MediaPlayer::SetOption(const char *key, const char *value)
    {
        GET_PLAYER_HANDLE
        CicadaSetOption(handle, key, value);
    }

    void MediaPlayer::GetOption(const char *key, char *value)
    {
        GET_PLAYER_HANDLE
        CicadaGetOption(handle, key, value);
    }

    void MediaPlayer::SetDefaultBandWidth(int bandWidth)
    {
        GET_PLAYER_HANDLE
        CicadaSetDefaultBandWidth(handle, bandWidth);
    }

    void MediaPlayer::preparedCallback(void *userData)
    {
        GET_MEDIA_PLAYER

        if (player->waitingForStart) {
            player->waitingForStart = false;
            player->Start();
        }

        if (player->mCollector) {
            player->mCollector->ReportPrepared();
        }

        if (player->waitingForLoop) {
        } else {
            if (player->mListener.Prepared) {
                player->mListener.Prepared(player->mListener.userData);
            }
        }
    }

    void MediaPlayer::completionCallback(void *userData)
    {
        GET_MEDIA_PLAYER
#ifdef ENABLE_CACHE_MODULE
        if (player->mCacheManager != nullptr) {
            CacheModule::CacheStatus cacheStatus = player->mCacheManager->getCacheStatus();
            bool isLoop = player->IsLoop();

            if (isLoop && cacheStatus == CacheModule::CacheStatus::success) {
                //If cacheSuccess and want to loop, reuse cache file.
                string sourceUrl = player->mCacheManager->getSourceUrl();
                player->Stop();
                player->waitingForStart = true;
                player->waitingForLoop = true;
                player->SetDataSource(sourceUrl.c_str());
                player->SetLoop(true);
                player->Prepare();

                if (player->mListener.LoopingStart) {
                    player->mListener.LoopingStart(player->mListener.userData);
                }

                return;
            }
        }
#endif
        if (player->mCollector) {
            player->mCollector->ReportCompletion();
        }

        if (player->mListener.Completion) {
            player->mListener.Completion(player->mListener.userData);
        }
    }

    void MediaPlayer::firstFrameCallback(void *userData)
    {
        GET_MEDIA_PLAYER

        if (player->mCollector) {
            player->mCollector->ReportFirstRender();
        }

        player->waitingForLoop = false;

        if (player->mListener.FirstFrameShow) {
            player->mListener.FirstFrameShow(player->mListener.userData);
        }
    }

    void MediaPlayer::errorFrameCallback(int64_t errorCode, const void *errorMsg, void *userData)
    {
        GET_MEDIA_PLAYER

        if (player->mOldPlayStatus < PLAYER_PREPARED) {
            //if play cache file , we try prepare the orignal url.
#ifdef ENABLE_CACHE_MODULE
            if (player->mCacheManager != nullptr) {
                string sourceUrl = player->mCacheManager->getSourceUrl();
                player->mCacheManager->stop("cache stopped by error");

                if (sourceUrl != player->mPlayUrl) {
                    //remove wrong cache file, and try play original url.
                    if (Cicada::FileUtils::rmrf(player->mPlayUrl.c_str()) == FILE_TRUE) {
                        player->SetDataSource(sourceUrl.c_str());
                        player->Prepare();
                        return;
                    }
                }
            }
#endif
        }

        if (player->mCollector) {
            player->mCollector->ReportError((int) errorCode, (char *) errorMsg, "");
        }

        if (player->mListener.ErrorCallback) {
            player->mListener.ErrorCallback(errorCode, errorMsg, player->mListener.userData);
        }
    }

    void MediaPlayer::eventCallback(int64_t code, const void *msg, void *userData)
    {
        GET_MEDIA_PLAYER

        if (code == MediaPlayerEventType::MEDIA_PLAYER_EVENT_SW_VIDEO_DECODER) {
            if (player->mCollector) {
                player->mCollector->ReportSwitchToSoftDecode();
            }
        } else if (code == MediaPlayerEventType::MEDIA_PLAYER_EVENT_DEMUXER_EOF) {
#ifdef ENABLE_CACHE_MODULE
            if (player->mCacheManager != nullptr) {
                player->mCacheManager->complete();
            }
#endif
        }

        if (player->mListener.EventCallback) {
            player->mListener.EventCallback(code, msg, player->mListener.userData);
        }
    }

    void MediaPlayer::videoSizeChangedCallback(int64_t width, int64_t height, void *userData)
    {
        GET_MEDIA_PLAYER

        if (player->mListener.VideoSizeChanged) {
            player->mListener.VideoSizeChanged(width, height, player->mListener.userData);
        }

        if (player->mCollector) {
            player->mCollector->ReportVideoSizeChanged(static_cast<int>(width), static_cast<int>(height));
        }
    }

    void MediaPlayer::videoRenderedCallback(int64_t timeMs, int64_t pts, void *userData)
    {
        GET_MEDIA_PLAYER

        if (player->mListener.VideoRendered) {
            player->mListener.VideoRendered(timeMs, pts, player->mListener.userData);
        }
    }


    void MediaPlayer::currentPostionCallback(int64_t position, void *userData)
    {
        GET_MEDIA_PLAYER

        if (player->mListener.PositionUpdate) {
            player->mListener.PositionUpdate(position, player->mListener.userData);
        }
    }

    void MediaPlayer::bufferPostionCallback(int64_t position, void *userData)
    {
        GET_MEDIA_PLAYER

        if (player->mListener.BufferPositionUpdate) {
            player->mListener.BufferPositionUpdate(position, player->mListener.userData);
        }
    }

    void MediaPlayer::loopingStartCallback(void *userData)
    {
        GET_MEDIA_PLAYER

        if (player->mListener.LoopingStart) {
            player->mListener.LoopingStart(player->mListener.userData);
        }

        if (player->mCollector) {
            player->mCollector->ReportLoopingStart();
        }
    }

    void MediaPlayer::loadingStartCallback(void *userData)
    {
        GET_MEDIA_PLAYER

        if (player->mListener.LoadingStart) {
            player->mListener.LoadingStart(player->mListener.userData);
        }

        if (player->mCollector) {
            player->mCollector->ReportLoadingStart();
        }
    }

    void MediaPlayer::loadingEndCallback(void *userData)
    {
        GET_MEDIA_PLAYER

        if (player->mListener.LoadingEnd) {
            player->mListener.LoadingEnd(player->mListener.userData);
        }

        if (player->mCollector) {
            player->mCollector->ReportLoadingEnd();
        }
    }

    void MediaPlayer::loadingProgressCallback(int64_t prg, void *userData)
    {
        GET_MEDIA_PLAYER

        if (player->mListener.LoadingProgress) {
            player->mListener.LoadingProgress(prg, player->mListener.userData);
        }
    }

    void MediaPlayer::subtitleShowCallback(int64_t index, int64_t size, const void *content,
                                           void *userData)
    {
        GET_MEDIA_PLAYER

        if (player->mListener.SubtitleShow) {
            player->mListener.SubtitleShow(index, size, content, player->mListener.userData);
        }
    }

    void MediaPlayer::subtitleHideCallback(int64_t index, int64_t size, const void *content, void *userData)
    {
        GET_MEDIA_PLAYER

        if (player->mListener.SubtitleHide) {
            player->mListener.SubtitleHide(index, size, content, player->mListener.userData);
        }
    }

    void MediaPlayer::subtitleExtAddedCallback(int64_t index, const void *url, void *userData)
    {
        GET_MEDIA_PLAYER

        if (player->mListener.SubtitleExtAdd) {
            player->mListener.SubtitleExtAdd(index, url, player->mListener.userData);
        }
    }

    void MediaPlayer::streamChangedSucCallback(int64_t type, const void *Info, void *userData)
    {
        GET_MEDIA_PLAYER
        StreamInfo *streamInfo = (StreamInfo *) Info;

        //when stream changed, set abr current video bitrate
        if (type == ST_TYPE_VIDEO) {
            player->mAbrAlgo->SetCurrentBitrate(streamInfo->videoBandwidth);
        }

        if (player->mListener.StreamSwitchSuc) {
            player->mListener.StreamSwitchSuc(type, Info, player->mListener.userData);
        }
    }

    void MediaPlayer::PlayerSeeking(int64_t seekInCache, void *userData)
    {
        GET_MEDIA_PLAYER
#ifdef ENABLE_CACHE_MODULE
        if (player->mCacheManager != nullptr && seekInCache == 0) {
                //not seek in cache
            player->mCacheManager->stop("cache stopped by seek");
        }
#endif
    }

    void MediaPlayer::PlayerSeekEnd(int64_t seekInCache, void *userData)
    {
        GET_MEDIA_PLAYER

        if (player->mListener.SeekEnd) {
            player->mListener.SeekEnd(seekInCache, player->mListener.userData);
        }

        //when seek, reset and open abrmanager
        player->mAbrManager->Reset();
        player->mAbrManager->Start();
        if (player->mCollector) {
            player->mCollector->ReportSeekEnd();
        }
    }

    void MediaPlayer::PlayerStatusChanged(int64_t oldStatus, int64_t newStatus, void *userData)
    {
        GET_MEDIA_PLAYER
        player->mOldPlayStatus = static_cast<PlayerStatus>(oldStatus);

        if (player->mListener.StatusChanged) {
            player->mListener.StatusChanged(oldStatus, newStatus, player->mListener.userData);
        }

        if (player->mCollector)
            player->mCollector->ReportPlayerStatueChange(static_cast<PlayerStatus>(oldStatus),
                                                         static_cast<PlayerStatus>(newStatus));
    }

    void MediaPlayer::captureScreenResult(int64_t width, int64_t height, const void *buffer, void *userData)
    {
        GET_MEDIA_PLAYER

        if (player->mListener.CaptureScreen) {
            player->mListener.CaptureScreen(width, height, buffer, player->mListener.userData);
        }
    }

    void MediaPlayer::autoPlayStart(void *userData)
    {
        GET_MEDIA_PLAYER

        if (player->mCollector) {
            player->mCollector->ReportPlay();
        }

        if (player->mListener.AutoPlayStart) {
            player->mListener.AutoPlayStart(player->mListener.userData);
        }

        player->mAbrManager->Start();
    }

    void MediaPlayer::streamInfoGetCallback(int64_t count, const void *Infos, void *userData)
    {
        GET_MEDIA_PLAYER
        //add video bitrate to abr manager
        StreamInfo **sInfos = (StreamInfo **) Infos;

        for (int i = 0; i < count; i++) {
            StreamInfo *si = sInfos[i];

            if (si->type == ST_TYPE_VIDEO) {
                player->mAbrAlgo->AddStreamInfo(si->streamIndex, si->videoBandwidth);
            }
        }

        player->mAbrAlgo->SetDuration(player->GetDuration());
        StreamInfo *si = player->GetCurrentStreamInfo(ST_TYPE_VIDEO);

        if (si) {
            player->mAbrAlgo->SetCurrentBitrate(si->videoBandwidth);

            if (player->mCollector) {
                player->mCollector->ReportCurrentBitrate(si->videoBandwidth);
            }
        }

        if (player->mListener.StreamInfoGet) {
            player->mListener.StreamInfoGet(count, Infos, player->mListener.userData);
        }
    }

    void MediaPlayer::setBitStreamCb(readCB read, seekCB seek, void *arg)
    {
        GET_PLAYER_HANDLE;
        return CicadaSetBitStreamCb(handle, read, seek, arg);
    }

    void MediaPlayer::setErrorConverter(ErrorConverter *converter)
    {
        GET_PLAYER_HANDLE;
        return CicadaSetErrorConverter(handle, converter);
    }

    void MediaPlayer::SetMediaFrameCb(playerMediaFrameCb func, void *arg)
    {
        mMediaFrameFunc = func;
        mMediaFrameArg = arg;
    }

    int MediaPlayer::GetCurrentStreamMeta(Stream_meta *meta, StreamType type)
    {
        GET_PLAYER_HANDLE;
        return CicadaGetCurrentStreamMeta(handle, meta, type);
    }

    void MediaPlayer::Reload()
    {
        GET_PLAYER_HANDLE;
        CicadaReload(handle);
    }

    void MediaPlayer::SetAutoPlay(bool bAutoPlay)
    {
        GET_PLAYER_HANDLE;
        CicadaSetAutoPlay(handle, bAutoPlay);
    }

    bool MediaPlayer::IsAutoPlay()
    {
        GET_PLAYER_HANDLE;
        return CicadaIsAutoPLay(handle);
    }

    void MediaPlayer::SetCacheConfig(const CacheConfig &config)
    {
#ifdef ENABLE_CACHE_MODULE
        if (!mCacheConfig.isSame(config)) {
            if (mCacheManager != nullptr) {
                mCacheManager->stop("cache stopped by change config");
            }

            mCacheConfig = config;
        }
#endif
    }

    string MediaPlayer::GetCachePathByURL(const string &URL)
    {
#ifdef ENABLE_CACHE_MODULE
        return CacheManager::getCachePath(URL, mCacheConfig);
#endif
        return "";
    }

    void MediaPlayer::AddExtSubtitle(const char *uri)
    {
        GET_PLAYER_HANDLE;
        CicadaAddExtSubtitle(handle, uri);
    }

    void MediaPlayer::SelectExtSubtitle(int index, bool select)
    {
        GET_PLAYER_HANDLE;
        CicadaSelectExtSubtitle(handle, index, select);
    }

    void MediaPlayer::SetDataSourceChangedCallback(function<void(const string &)> urlChangedCallbak)
    {
        mPlayUrlChangedCallback = urlChangedCallbak;
    }

    void MediaPlayer::onMediaFrameCallback(void *arg, const unique_ptr<IAFPacket> &frame, StreamType type)
    {
        MediaPlayer *player = (MediaPlayer *) arg;

        if (nullptr == player) {
            return;
        }

        player->mediaFrameCallback(frame, type);
    }

    void MediaPlayer::mediaFrameCallback(const unique_ptr<IAFPacket> &frame, StreamType type)
    {
#ifdef ENABLE_CACHE_MODULE
        if (mCacheManager) {
            mCacheManager->sendMediaFrame(frame, type);
        }
#endif
        if (mMediaFrameFunc) {
            mMediaFrameFunc(mMediaFrameArg, frame, type);
        }
    }


    void MediaPlayer::EnableVideoRenderedCallback(bool enable)
    {
        GET_PLAYER_HANDLE;
        CicadaSetOption(handle, "enableVRC", enable ? "1" : "0");
    }

    void MediaPlayer::SetOnRenderFrameCallback(onRenderFrame cb, void *userData)
    {
        GET_PLAYER_HANDLE;
        CicadaSetOnRenderCallBack(handle, cb, userData);
    }
    void MediaPlayer::SetStreamTypeFlags(uint64_t flags)
    {
        GET_PLAYER_HANDLE;
        CicadaSetOption(handle, "streamTypes", to_string(flags).c_str());
    }
}
