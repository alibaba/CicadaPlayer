#define LOG_TAG "ApsaraPlayerService"

#include <utils/frame_work_log.h>
#include <utils/timer.h>
#include <cassert>
#include "SuperMediaPlayer.h"
#include "utils/CicadaJSON.h"
#include "utils/CicadaUtils.h"
#include <cinttypes>
#include <utils/err.h>
#include <utils/errors/framework_error.h>
#include "media_player_error_def.h"
#include <utils/oscl/oscl_utils.h>
#include <utils/ffmpeg_utils.h>
#include "media_player_error_map.h"
#include <utils/AFMediaType.h>
#include <utils/property.h>
#include <codec/avcodecDecoder.h>
#include <render/renderFactory.h>
#include <codec/decoderFactory.h>
#include <utils/file/FileUtils.h>
#include <data_source/dataSourcePrototype.h>
#include <demuxer/IDemuxer.h>
#include <utils/af_string.h>


#ifdef __APPLE__

    #include <TargetConditionals.h>
    #include <render/audio/Apple/AFAudioSessionWrapper.h>

#endif

#define PTS_DISCONTINUE_DELTA (20*1000*1000)
#define VIDEO_PICTURE_MAX_CACHE_SIZE 2

static int MAX_DECODE_ERROR_FRAME = 100;

//#define IS_REAL_TIME_STREAM (mSet.url.substr(0,7) == "rtmp://" || mSet.url.substr(0,7) == "artp://")
#define IS_REAL_TIME_STREAM ((mDuration == 0) && !(mDemuxerService->isPlayList()))


#define PTS_REVERTING (mVideoPtsRevert != mAudioPtsRevert)

namespace Cicada {

    static MsgParam dummyMsg{{nullptr}};

    const int64_t SuperMediaPlayer::SEEK_ACCURATE_MAX = 11 * 1000 * 1000;

    SuperMediaPlayer::SuperMediaPlayer()
        : mMessageControl(*this),
          mAudioRenderCB(*this),
          mApsaraThread([this]() -> int { return this->mainService(); }, LOG_TAG),
          mSourceListener(*this)
    {
        AF_LOGD("SuperMediaPlayer()");
        mPNotifier = new PlayerNotifier();
        Reset();
        mTimerInterval = 500;
    }

    SuperMediaPlayer::~SuperMediaPlayer()
    {
        Stop();
        AF_LOGD("SuperMediaPlayer");
        mCanceled = true;
        mPlayerCondition.notify_one();
        mApsaraThread.stop();
        mVideoRender = nullptr;
        mSubPlayer = nullptr;
        mSubListener = nullptr;
        // delete mPNotifier after mPMainThread, to avoid be using
        delete mPNotifier;
        mPNotifier = nullptr;
    }

    void SuperMediaPlayer::putMsg(PlayMsgType type, const MsgParam &param, bool trigger)
    {
        mMessageControl.putMsg(type, param);

        if (trigger) {
            mPlayerCondition.notify_one();
        }
    }

    void SuperMediaPlayer::SetView(void *view)
    {
        ProcessSetViewMsg(view);
    }

    int64_t SuperMediaPlayer::GetMasterClockPts()
    {
        return mMasterClock.GetTime();
    }

    void SuperMediaPlayer::SetDataSource(const char *url)
    {
        MsgParam param;
        MsgDataSourceParam dataSourceParam = {nullptr};
        dataSourceParam.url = new string(url ? url : "");
        param.dataSourceParam = dataSourceParam;
        putMsg(MSG_SETDATASOURCE, param);
    }

    void SuperMediaPlayer::Prepare()
    {
        if (mPlayStatus != PLAYER_INITIALZED && mPlayStatus != PLAYER_STOPPED) {
            Stop();
        }

#if TARGET_OS_IPHONE
        AFAudioSessionWrapper::activeAudio();
#endif
        mPrepareStartTime = af_gettime_relative();
        std::unique_lock<std::mutex> uMutex(mPlayerMutex);
        putMsg(MSG_PREPARE, dummyMsg);
        mApsaraThread.start();
    }

    void SuperMediaPlayer::CaptureScreen()
    {
        std::lock_guard<std::mutex> uMutex(mCreateMutex);

        if (mVideoRender && mCurrentVideoIndex >= 0) {
            mVideoRender->captureScreen([this](uint8_t *data, int width, int height) {
                if (this->mPNotifier) {
                    this->mPNotifier->NotifyCaptureScreen(data, width, height);
                }
            });
        } else {
            if (this->mPNotifier) {
                this->mPNotifier->NotifyCaptureScreen(nullptr, 0, 0);
            }
        }
    }

    void SuperMediaPlayer::SetVolume(float volume)
    {
        //TODO:put message to
        mSet.mVolume = volume;

        if (mSet.mVolume < 0) {
            mSet.mVolume = 0;
        } else if (mSet.mVolume > 1.0) {
            AF_LOGW("volume >1.0");
        }

        if (mAudioRender != nullptr) {
            mAudioRender->setVolume(mSet.mVolume);
        }
    }

    void SuperMediaPlayer::Start()
    {
        if ((PLAYER_INITIALZED == mPlayStatus) || (PLAYER_PREPARING == mPlayStatus) || PLAYER_PREPARINIT == mPlayStatus) {
            waitingForStart = true;
        }

        this->putMsg(MSG_START, dummyMsg);
    }


    void SuperMediaPlayer::Pause()
    {
        waitingForStart = false;
        this->putMsg(MSG_PAUSE, dummyMsg);
    }


    void SuperMediaPlayer::SeekTo(int64_t pos, bool bAccurate)
    {
        MsgParam param;
        MsgSeekParam seekParam;
        seekParam.seekPos = (int64_t) pos * 1000;
        seekParam.bAccurate = bAccurate;
        param.seekParam = seekParam;
        this->putMsg(MSG_SEEKTO, param);
        mSeekPos = pos * 1000;
        mSeekNeedCatch = bAccurate;
    }

    bool SuperMediaPlayer::OnPlayerMsgIsPadding(PlayMsgType msg, MsgParam msgContent)
    {
        bool padding = false;

        switch (msg) {
            case MSG_CHANGE_VIDEO_STREAM:
                padding = mVideoChangedFirstPts != INT64_MIN;
                break;

            case MSG_CHANGE_AUDIO_STREAM:
                padding = mAudioChangedFirstPts != INT64_MIN;
                break;

            case MSG_CHANGE_SUBTITLE_STREAM:
                padding = mSubtitleChangedFirstPts != INT64_MIN;
                break;

            case MSG_SEEKTO:
                if (mSeekFlag) {
                    padding = true;
                }

                break;

            default:
                padding = false;
        }

        return padding;
    }

    void SuperMediaPlayer::Mute(bool bMute)
    {
        if (bMute == mSet.bMute) {
            return;
        }

        mSet.bMute = bMute;
        this->putMsg(MSG_MUTE, dummyMsg);
    }

    void SuperMediaPlayer::EnterBackGround(bool back)
    {
        // lock mAppStatusMutex before mCreateMutex
        std::lock_guard<std::mutex> lock(mAppStatusMutex);
        MsgParam param;
        MsgHoldOnVideoParam holdParam;

        if (back) {
            AF_LOGI("EnterBackGround");
            mAppStatus = APP_BACKGROUND;
            holdParam.hold = true;
            param.msgHoldOnVideoParam = holdParam;
            putMsg(MSG_INTERNAL_VIDEO_HOLD_ON, param);

            if (mPlayStatus == PLAYER_PLAYING) {
                putMsg(MSG_INTERNAL_VIDEO_CLEAN_FRAME, dummyMsg);
            }
        } else {
            AF_LOGI("EnterBackGround APP_FOREGROUND");
            mAppStatus = APP_FOREGROUND;
            holdParam.hold = false;
            param.msgHoldOnVideoParam = holdParam;
            putMsg(MSG_INTERNAL_VIDEO_HOLD_ON, param);
        }
    }

    StreamType SuperMediaPlayer::SwitchStream(int streamIndex)
    {
        MsgParam param;
        MsgChangeStreamParam streamParam;
        streamParam.index = streamIndex;
        param.streamParam = streamParam;
        StreamType streamType = ST_TYPE_UNKNOWN;
        PlayMsgType type = MSG_INVALID;

        for (auto &it : mStreamInfoQueue) {
            if (it->streamIndex == streamIndex) {
                switch (it->type) {
                    case ST_TYPE_VIDEO:
                        streamType = ST_TYPE_VIDEO;
                        type = MSG_CHANGE_VIDEO_STREAM;
                        break;

                    case ST_TYPE_AUDIO:
                        streamType = ST_TYPE_AUDIO;
                        type = MSG_CHANGE_AUDIO_STREAM;
                        break;

                    case ST_TYPE_SUB:
                        streamType = ST_TYPE_SUB;
                        type = MSG_CHANGE_SUBTITLE_STREAM;
                        break;

                    default:
                        AF_LOGE("unknown stream Type");
                        return streamType;
                }

                break;
            }
        }

        if (type != MSG_INVALID) {
            this->putMsg(type, param);
        }

        return streamType;
    }

    void SuperMediaPlayer::Interrupt(bool inter)
    {
        AF_TRACE;
        std::lock_guard<std::mutex> locker(mCreateMutex);

        if (mDataSource) {
            mDataSource->Interrupt(inter);
        } else {
            AF_TRACE;
        }

        if (mDemuxerService) {
            mDemuxerService->interrupt(inter);
            mDemuxerService->preStop();
        } else {
            AF_TRACE;
        }
    }


    int SuperMediaPlayer::Stop()
    {
        if ((afThread::THREAD_STATUS_RUNNING != mApsaraThread.getStatus())
                && ((mPlayStatus == PLAYER_IDLE) || (mPlayStatus == PLAYER_STOPPED))) {
            return 0;
        }

        std::unique_lock<std::mutex> uMutex(mPlayerMutex);
        AF_LOGI("Player ReadPacket Stop");
        int64_t t1 = af_gettime_ms();
        AF_TRACE;
        waitingForStart = false;
        mCanceled = true;
        mPNotifier->Clean();
        mPNotifier->Enable(false);
        Interrupt(true);
        mPlayerCondition.notify_one();
        mApsaraThread.pause();
        mPlayStatus = PLAYER_STOPPED;
        //        ChangePlayerStatus(PLAYER_STOPPED);
        mBufferController.ClearPacket(BUFFER_TYPE_AV);
        AF_TRACE;

        if (mVideoDecoder) {
            mVideoDecoder->preClose();
        }

        AF_TRACE;

        if (mAudioDecoder != nullptr) {
            //because FlushAudioPath call mAudioOutHandle flush, so stop should called first
            if (mAudioRender) {
                mAudioRender->pause(true);
            }

            FlushAudioPath();
            mAudioDecoder->preClose();
        }

        AF_TRACE;
        mAudioRender = nullptr;
        mBRendingStart = false;
        AF_TRACE;
        {
            std::lock_guard<std::mutex> uMutex(mCreateMutex);

            if (mVideoDecoder) {
                FlushVideoPath();
                mVideoDecoder->close();
                mVideoDecoder = nullptr;
            }

            if (mAudioDecoder) {
                FlushAudioPath();
                mAudioDecoder->close();
                mAudioDecoder = nullptr;
            }
        }
        // clear the message queue after flash video render
        mMessageControl.clear();
        AF_TRACE;

        if (mDemuxerService) {
            mDemuxerService->interrupt(1);

            if (mDataSource) {
                mDataSource->Interrupt(true);
            }

            std::lock_guard<std::mutex> uMutex(mCreateMutex);
            mDemuxerService->stop();
            mDemuxerService->close();

            if (mMixMode) {
                if (mMainStreamId != -1) {
                    mDemuxerService->CloseStream(mMainStreamId);
                }

                if (mCurrentSubtitleIndex >= 0) {
                    mDemuxerService->CloseStream(mCurrentSubtitleIndex);
                }
            } else {
                if (mCurrentAudioIndex >= 0) {
                    mDemuxerService->CloseStream(mCurrentAudioIndex);
                }

                if (mCurrentVideoIndex >= 0) {
                    mDemuxerService->CloseStream(mCurrentVideoIndex);
                }

                if (mCurrentSubtitleIndex >= 0) {
                    mDemuxerService->CloseStream(mCurrentSubtitleIndex);
                }
            }

            delete mDemuxerService;
            mDemuxerService = nullptr;
        }

        if (mDataSource) {
            mDataSource->Close();
            std::lock_guard<std::mutex> uMutex(mCreateMutex);
            delete mDataSource;
            mDataSource = nullptr;
        }

        if (mVideoRender) {
            // lock mAppStatusMutex before mCreateMutex
            std::lock_guard<std::mutex> lock(mAppStatusMutex);

            // for iOS, don't delete render in background, and we should reuse it later.
            if (APP_BACKGROUND != mAppStatus) {
                std::lock_guard<std::mutex> uMutex(mCreateMutex);

                if (mSet.clearShowWhenStop) {
                    mVideoRender->clearScreen();
                }
            }
        }

        delete[] mStreamInfos;
        mStreamInfos = nullptr;
        delete mVideoParser;
        mVideoParser = nullptr;
        mSubPlayer = nullptr;
        {
            std::lock_guard<std::mutex> uMutex(mCreateMutex);

            for (StreamInfo *info : mStreamInfoQueue) {
                releaseStreamInfo(info);
            }

            mStreamInfoQueue.clear();
        }
        mBufferController.ClearPacket(BUFFER_TYPE_SUBTITLE);
        Reset();
        AF_LOGD("stop spend time is %lld", af_gettime_ms() - t1);
        return 0;
    }

    void SuperMediaPlayer::releaseStreamInfo(const StreamInfo *info) const
    {
        if (info->subtitleLang) {
            free(info->subtitleLang);
        }

        if (info->audioLang) {
            free(info->audioLang);
        }

        if (info->description) {
            free(info->description);
        }

        delete info;
    }

    void SuperMediaPlayer::SetRefer(const char *referer)
    {
        if (referer) {
            mSet.refer = referer;
        }
    }

    void SuperMediaPlayer::SetUserAgent(const char *userAgent)
    {
        if (userAgent) {
            mSet.userAgent = userAgent;
        }
    }

    void SuperMediaPlayer::SetTimeout(int timeout)
    {
        mSet.timeout_ms = timeout;
    }

    void SuperMediaPlayer::SetDropBufferThreshold(int dropValue)
    {
        mSet.RTMaxDelayTime = dropValue * 1000;
    }

    void SuperMediaPlayer::SetLooping(bool looping)
    {
        mSet.bLooping = looping;
    }

    bool SuperMediaPlayer::isLooping()
    {
        return mSet.bLooping;
    }

    int SuperMediaPlayer::SetOption(const char *key, const char *value)
    {
        if (key == nullptr) {
            return -1;
        }

        int duration;
        string theKey = key;

        if (theKey == "startBufferDuration") {
            duration = atoi(value);

            if (duration > 0) {
                mSet.startBufferDuration = duration * 1000;
            }
        } else if (theKey == "RTMaxDelayTime") {
            duration = atoi(value);

            if (duration > 0) {
                mSet.RTMaxDelayTime = duration * 1000;
            }
        } else if (theKey == "highLevelBufferDuration") {
            duration = atoi(value);

            if (duration > 0) {
                mSet.highLevelBufferDuration = duration * 1000;
            }
        } else if (theKey == "http_proxy") {
            mSet.http_proxy = value;
        } else if (theKey == "maxBufferDuration") {
            duration = atoi(value);

            if (duration > 0) {
                mSet.maxBufferDuration = int64_t(duration) * 1000;
            }
        } else if (theKey == "DisableBufferManager") {
            mSet.bDisableBufferManager = (bool) atoi(value);
        } else if (theKey == "LowLatency") {
            mSet.bLowLatency = (bool) atoi(value);
        } else if (theKey == "ClearShowWhenStop") {
            int clearShowWhenStop = atoi(value);
            mSet.clearShowWhenStop = (bool) clearShowWhenStop;
        } else if (theKey == "enableVideoTunnelRender") {
            mSet.bEnableTunnelRender = (atoi(value) != 0);
        } else if (theKey == "Analytics.ReportID") {
            if (nullptr == value) {
                return -1;
            }

            int64_t eventReportID = atoll(value);
            mSet.AnalyticsID = eventReportID;
        } else if (theKey == "bandWidth") {
            mSet.mDefaultBandWidth = atoi(value);
        } else if (theKey == "description") {
            mSet.mOptions.set(theKey, value, options::REPLACE);
            return 0;
        } else if (theKey == "enableVRC") {
            mSet.bEnableVRC = (atoi(value) != 0);
        } else if (theKey == "maxAccurateSeekDelta") {
            mSet.maxASeekDelta = atoi(value) * 1000;
        } else if (theKey == "maxVideoRecoverSize") {
            mSet.maxVideoRecoverSize = atoi(value);
        } else if ( theKey == "surfaceChanged") {
            std::lock_guard<std::mutex> uMutex(mCreateMutex);

            if (mVideoRender != nullptr) {
                mVideoRender->surfaceChanged();
            }
        } else if (theKey == "streamTypes") {
            uint64_t flags = atoll(value);
            mSet.bDisableAudio = mSet.bDisableVideo = true;
            if (flags & VIDEO_FLAG) {
                mSet.bDisableVideo = false;
            }
            if (flags & AUDIO_FLAG) {
                mSet.bDisableAudio = false;
            }
        }

        return 0;
    }

    void SuperMediaPlayer::GetOption(const char *key, char *value)
    {
        if (key == nullptr) {
            return;
        }

        string theKey = key;

        if (theKey == "maxBufferDuration") {
            snprintf(value, MAX_OPT_VALUE_LENGTH, "%" PRId64 "", mSet.maxBufferDuration);
        } else if (theKey == "mediaStreamSize") {
            int64_t size = -1;
            std::unique_lock<std::mutex> uMutex(mCreateMutex);

            if (mDataSource && mDemuxerService) {
                if (!mDemuxerService->isPlayList()) {
                    size = mDataSource->Seek(0, SEEK_SIZE);
                }
            }

            snprintf(value, MAX_OPT_VALUE_LENGTH, "%" PRId64 "", size);
        } else if (theKey == "description") {
            sprintf(value, "%s", mSet.mOptions.get("description").c_str());
        } else if (theKey == "descriptionLen") {
            snprintf(value, MAX_OPT_VALUE_LENGTH, "%lu",
                     static_cast<unsigned long>(mSet.mOptions.get("description").length()));
        } else if (theKey == "renderFps") {
            float renderFps = GetVideoRenderFps();
            snprintf(value, MAX_OPT_VALUE_LENGTH, "%f", renderFps);
        }

        return;
    }

    int64_t SuperMediaPlayer::GetCurrentPosition()
    {
        return getCurrentPosition() / 1000;
    }

    void SuperMediaPlayer::NotifyPosition(int64_t position)
    {
        mPlayingPosition = position;
        mPNotifier->NotifyPosition(position);
    }

    int64_t SuperMediaPlayer::GetPlayingPosition()
    {
        if (isSeeking()) {
            return mSeekPos / 1000;
        }

        return mPlayingPosition;
    }

    int64_t SuperMediaPlayer::getCurrentPosition()
    {
        if (isSeeking()) {
            return mSeekPos;
        }

        mCurrentPos = mCurrentPos < 0 ? 0 : mCurrentPos;

        if (mDuration > 0) {
            mCurrentPos = mCurrentPos <= mDuration ? mCurrentPos : mDuration;
        }

        return mCurrentPos;
    }

    void SuperMediaPlayer::SetScaleMode(ScaleMode mode)
    {
        if (mode == mSet.scaleMode) {
            return;
        }

        mSet.scaleMode = static_cast<ScaleMode>(mode);
        this->putMsg(MSG_SET_DISPLAY_MODE, dummyMsg);
    }

    void SuperMediaPlayer::SetRotateMode(RotateMode mode)
    {
        if (mode == mSet.rotateMode) {
            return;
        }

        mSet.rotateMode = static_cast<RotateMode>(mode);
        this->putMsg(MSG_SET_ROTATE_MODE, dummyMsg);
    }

    RotateMode SuperMediaPlayer::GetRotateMode()
    {
        return mSet.rotateMode;
    }

    void SuperMediaPlayer::SetMirrorMode(MirrorMode mode)
    {
        if (mode == mSet.mirrorMode) {
            return;
        }

        mSet.mirrorMode = static_cast<MirrorMode>(mode);
        this->putMsg(MSG_SET_MIRROR_MODE, dummyMsg);
    }

    void SuperMediaPlayer::SetVideoBackgroundColor(uint32_t color)
    {
        if (color == mSet.mVideoBackgroundColor ) {
            return;
        }

        mSet.mVideoBackgroundColor = color;
        this->putMsg(MSG_SET_VIDEO_BACKGROUND_COLOR, dummyMsg);
    }

    MirrorMode SuperMediaPlayer::GetMirrorMode()
    {
        return mSet.mirrorMode;
    }

    ScaleMode SuperMediaPlayer::GetScaleMode()
    {
        return mSet.scaleMode;
    }

    int64_t SuperMediaPlayer::GetBufferPosition()
    {
        return mBufferPosition / 1000;
    }

    int64_t SuperMediaPlayer::GetDuration() const
    {
        return mDuration / 1000;
    }

    // TODO: change name to EnableHwDecode
    void SuperMediaPlayer::SetDecoderType(DecoderType type)
    {
        mSet.bEnableHwVideoDecode = (type == DT_HARDWARE);
    }

    void SuperMediaPlayer::AddCustomHttpHeader(const char *header)
    {
        for (auto &item : mSet.customHeaders) {
            if (item == header) {
                return;
            }
        }

        mSet.customHeaders.emplace_back(header);
    }

    void SuperMediaPlayer::RemoveAllCustomHttpHeader()
    {
        mSet.customHeaders.clear();
    }

    // TODO: move to mainService thread
    void SuperMediaPlayer::setSpeed(float speed)
    {
        MsgParam param;
        MsgSpeedParam speedParam;
        speedParam.speed = speed;
        param.msgSpeedParam = speedParam;
        putMsg(MSG_SET_SPEED, param);
    }

    float SuperMediaPlayer::getSpeed()
    {
        return mSet.rate;
    }

    DecoderType SuperMediaPlayer::GetDecoderType()
    {
        std::lock_guard<std::mutex> uMutex(mCreateMutex);

        if (mVideoDecoder) {
            if (mVideoDecoder->getFlags() & DECFLAG_HW) {
                return DT_HARDWARE;
            }
        }

        return DT_SOFTWARE;
    }

    PlayerStatus SuperMediaPlayer::GetPlayerStatus() const
    {
        return mPlayStatus;
    }

    float SuperMediaPlayer::GetVolume() const
    {
        return mSet.mVolume;
    }

    int64_t SuperMediaPlayer::GetPropertyInt(PropertyKey key)
    {
        switch (key) {
            case PROPERTY_KEY_VIDEO_BUFFER_LEN: {
                int64_t duration = mBufferController.GetPacketDuration(BUFFER_TYPE_VIDEO);

                if (duration < 0) {
                    duration = mBufferController.GetPacketLastPTS(BUFFER_TYPE_VIDEO) -
                               mBufferController.GetPacketPts(BUFFER_TYPE_VIDEO);
                }

                return duration;
            }

            case PROPERTY_KEY_REMAIN_LIVE_SEG:
                return mRemainLiveSegment;

            case PROPERTY_KEY_NETWORK_IS_CONNECTED:
                return mSourceListener.isConnected();

            default:
                break;
        }

        return 0;
    }

    std::string SuperMediaPlayer::GetPropertyString(PropertyKey key)
    {
        switch (key) {
            case PROPERTY_KEY_RESPONSE_INFO: {
                CicadaJSONArray array;
                std::lock_guard<std::mutex> uMutex(mCreateMutex);
                MediaPlayerUtil::addURLProperty("responseInfo", array, mDataSource);
                //if (mDemuxerService->isPlayList())
                {
                    MediaPlayerUtil::getPropertyJSONStr("responseInfo", array, false, mStreamInfoQueue,
                                                        mDemuxerService);
                }
                return array.printJSON();
            }

            case PROPERTY_KEY_CONNECT_INFO: {
                std::lock_guard<std::mutex> uMutex(mCreateMutex);

                if (mDataSource) {
                    return mDataSource->GetOption("connectInfo");
                }

                return "";
            }

            case PROPERTY_KEY_OPEN_TIME_STR: {
                CicadaJSONArray array;
                CicadaJSONItem item;
                item.addValue("readpacketMS", (double) mFirstReadPacketSucMS);
                array.addJSON(item);
                std::lock_guard<std::mutex> uMutex(mCreateMutex);
                MediaPlayerUtil::addURLProperty("connectInfo", array, mDataSource);
                //if (mDemuxerService->isPlayList())
                {
                    MediaPlayerUtil::getPropertyJSONStr("openJsonInfo", array, true, mStreamInfoQueue,
                                                        mDemuxerService);
                }
                return array.printJSON();
            }

            case PROPERTY_KEY_PROBE_STR: {
                CicadaJSONArray array;
                std::lock_guard<std::mutex> uMutex(mCreateMutex);
                MediaPlayerUtil::addURLProperty("probeInfo", array, mDataSource);

                if (nullptr == mDemuxerService) {
                    return array.printJSON();
                } else if (mDemuxerService->isPlayList()) {
                    MediaPlayerUtil::getPropertyJSONStr("probeInfo", array, false, mStreamInfoQueue, mDemuxerService);
                } else {
                    CicadaJSONItem item(mDemuxerService->GetProperty(0, "probeInfo"));
                    item.addValue("type", "video");
                    array.addJSON(item);
                }

                return array.printJSON();
            }

            case PROPERTY_KEY_DELAY_INFO: {
                if (nullptr != mDemuxerService) {
                    string ret = mDemuxerService->GetProperty(0, "delayInfo");
                    return ret;
                }

                return "";
            }

            default:
                break;
        }

        return "";
    }

    int SuperMediaPlayer::getCurrentStreamMeta(Stream_meta *meta, StreamType type)
    {
        int streamIndex = -1;

        switch (type) {
            case ST_TYPE_VIDEO:
                streamIndex = mCurrentVideoIndex;
                break;

            case ST_TYPE_AUDIO:
                streamIndex = mCurrentAudioIndex;
                break;

            case ST_TYPE_SUB:
                streamIndex = mCurrentSubtitleIndex;
                break;

            default:
                return -EINVAL;
        }

        std::unique_lock<std::mutex> uMutex(mCreateMutex);

        if (streamIndex < 0 || mDemuxerService == nullptr) {
            return -EINVAL;
        }

        return mDemuxerService->GetStreamMeta(meta, streamIndex, false);
    }

    void SuperMediaPlayer::reLoad()
    {
        mSourceListener.enableRetry();
        std::lock_guard<std::mutex> uMutex(mCreateMutex);
        if (mDemuxerService) {
            mDemuxerService->getDemuxerHandle()->Reload();
        }
    }

    IVideoRender::Scale SuperMediaPlayer::convertScaleMode(ScaleMode mode)
    {
        if (mode == ScaleMode::SM_CROP) {
            return IVideoRender::Scale::Scale_AspectFill;
        } else if (mode == ScaleMode::SM_FIT) {
            return IVideoRender::Scale::Scale_AspectFit;
        } else {
            return IVideoRender::Scale::Scale_Fill;
        }
    }

    IVideoRender::Rotate SuperMediaPlayer::convertRotateMode(RotateMode mode)
    {
        if (mode == RotateMode::ROTATE_MODE_0) {
            return IVideoRender::Rotate::Rotate_None;
        } else if (mode == RotateMode::ROTATE_MODE_90) {
            return IVideoRender::Rotate::Rotate_90;
        } else if (mode == RotateMode::ROTATE_MODE_180) {
            return IVideoRender::Rotate::Rotate_180;
        } else if (mode == RotateMode::ROTATE_MODE_270) {
            return IVideoRender::Rotate::Rotate_270;
        } else {
            return IVideoRender::Rotate::Rotate_None;
        }
    }


    IVideoRender::Flip SuperMediaPlayer::convertMirrorMode(MirrorMode mode)
    {
        if (mode == MirrorMode::MIRROR_MODE_HORIZONTAL) {
            return IVideoRender::Flip::Flip_Horizontal;
        } else if (mode == MirrorMode::MIRROR_MODE_VERTICAL) {
            return IVideoRender::Flip::Flip_Vertical;
        } else if (mode == MirrorMode::MIRROR_MODE_NONE) {
            return IVideoRender::Flip::Flip_None;
        } else {
            return IVideoRender::Flip::Flip_None;
        }
    }

    float SuperMediaPlayer::GetVideoRenderFps()
    {
        if (mVideoRender) {
            return mVideoRender->getRenderFPS();
        }

        return mUtil.getVideoRenderFps();
    }


    float SuperMediaPlayer::GetVideoDecodeFps()
    {
        return 0.0f;
    }

    void SuperMediaPlayer::NotifyError(int code)
    {
        ChangePlayerStatus(PLAYER_ERROR);

        if (mErrorConverter) {
            int newErrorCode;
            std::string outStr;
            int processed = mErrorConverter->ConvertErrorCode(code, newErrorCode, outStr);

            if (processed) {
                AF_LOGE("Player ConvertErrorCode 0x%08x :%s\n", newErrorCode, outStr.c_str());
                mPNotifier->NotifyError(newErrorCode, outStr.c_str());
                return;
            }
        }

        int newErrorCode = framework_error2_code(code);
        char errbuf[128] = {0};
        int isFfmpegError = -1;

        if (newErrorCode == MEDIA_PLAYER_ERROR_UNKNOWN) {
            //maybe ffmpeg error code,,, try get ffmpeg error msg.
            isFfmpegError = get_ffmpeg_error_message(code, errbuf, 128);
        }

        if (isFfmpegError == 0) {
            AF_LOGE("Player ReadPacket ffmpeg error ?? 0x%04x :%s\n", -code, errbuf);
            mPNotifier->NotifyError(MEDIA_PLAYER_ERROR_DEMUXER_OPENSTREAM, errbuf);
        } else {
            char *desc = const_cast<char *>(framework_err2_string(code));
            mPNotifier->NotifyError(newErrorCode, desc);
        }
    }

    void SuperMediaPlayer::updateLoopGap()
    {
        switch (mPlayStatus.load()) {
            case PLAYER_PREPARINIT:
            case PLAYER_PREPARING:
            case PLAYER_PREPARED:
                mMaxRunningLoopGap = 3;
                break;

            case PLAYER_PLAYING:
                if (!mFirstRendered) {
                    mMaxRunningLoopGap = 3;
                } else {
                    if (mSet.bDisableBufferManager) {
                        if (CicadaUtils::isEqual(mSet.rate, 1.0)) {
                            mMaxRunningLoopGap = 8;
                        } else {
                            mMaxRunningLoopGap = 6;
                        }
                    } else {
                        if (dropLateVideoFrames || mSeekNeedCatch) {
                            mMaxRunningLoopGap = 2;
                            return;
                        }

                        float fps = GetVideoRenderFps();

                        if (fps > 20 && (mSet.rate < 1.3)) {
                            mMaxRunningLoopGap = static_cast<int>(1000 / fps - 5);
                        } else {
                            mMaxRunningLoopGap = 15;
                        }

                        //FIXME: for now this logic is not good, close it temp
                        mMaxRunningLoopGap = 10;

                        if (mLastAudioFrameDuration > 0) {
                            float duration = mLastAudioFrameDuration;

                            if (mSet.rate >= 0.5) {
                                // loop once will decoder and render two audio frame, set 1.5 for more safe
                                duration = static_cast<float>(
                                               ((float) mLastAudioFrameDuration * 1.5) /
                                               (1000 * mSet.rate));
                            }

                            if (mMaxRunningLoopGap > duration) {
                                mMaxRunningLoopGap = static_cast<int>(duration);
                            }
                        }

                        if (mMaxRunningLoopGap > 25) {
                            mMaxRunningLoopGap = 25;
                        } else if (mMaxRunningLoopGap < 10) {
                            mMaxRunningLoopGap = 10;
                        }
                    }
                }

                break;

            default:
                mMaxRunningLoopGap = 40;
                break;
        }
    }

    int SuperMediaPlayer::mainService()
    {
        int64_t curTime = af_gettime_relative();
        mUtil.notifyPlayerLoop(curTime);

        if (mMessageControl.empty() || (0 == mMessageControl.processMsg())) {
            ProcessVideoLoop();
            int64_t use = (af_gettime_relative() - curTime) / 1000;
            int64_t needWait = mMaxRunningLoopGap - use;
//            AF_LOGD("use :%lld, needWait:%lld", use, needWait);

            if (needWait <= 0) {
                if (mMaxRunningLoopGap < 5) {
                    needWait = 2;
                } else {
                    return 0;
                }
            }

            std::unique_lock<std::mutex> uMutex(mSleepMutex);
            mPlayerCondition.wait_for(uMutex, std::chrono::milliseconds(needWait),
            [this]() { return this->mCanceled.load(); });
        }

        return 0;
    }

    void SuperMediaPlayer::ProcessVideoLoop()
    {
        int64_t curTime = af_gettime_relative() / 1000;
        {
#ifndef NDEBUG
            int streamIds[] = {
                mCurrentVideoIndex,
                mCurrentAudioIndex,
                mCurrentSubtitleIndex,
                mWillChangedVideoStreamIndex,
                mWillChangedAudioStreamIndex,
                mWillChangedSubtitleStreamIndex
            };
            int size = sizeof(streamIds) / sizeof(streamIds[0]);

            for (int i = 0; i < size; ++i) {
                for (int j = 0; j < size; ++j) {
                    if (i != j && streamIds[i] >= 0) {
                        assert(streamIds[i] != streamIds[j]);
                    }
                }
            }

#endif
        }

        if (mSubPlayer) {
            mSubPlayer->onNoop();
        }

        if ((PLAYER_COMPLETION != mPlayStatus && (mPlayStatus < PLAYER_PREPARING || mPlayStatus > PLAYER_PAUSED))
                || nullptr == mDemuxerService) { // not working
            if (curTime - mTimerLatestTime > mTimerInterval) {
                OnTimer(curTime);
                mTimerLatestTime = curTime;
            }

            return;
        }

        setUpAVPath();
        doReadPacket();

        if (!DoCheckBufferPass()) {
            return;
        }

        doDeCode();

        if (!mBRendingStart && mPlayStatus == PLAYER_PLAYING && !mBufferingFlag) {
            if ((!HAVE_VIDEO || !mVideoFrameQue.empty() || (APP_BACKGROUND == mAppStatus))
                    && (!HAVE_AUDIO || !mAudioFrameQue.empty())) {
                startRendering(true);
            }
        }

        doRender();
        checkEOS();
        curTime = af_gettime_relative() / 1000;

        if (curTime - mTimerLatestTime > mTimerInterval) {
            OnTimer(curTime);
            mTimerLatestTime = curTime;
        }
    }

    void SuperMediaPlayer::doReadPacket()
    {
        //check packet queue full
        int64_t cur_buffer_duration = getPlayerBufferDuration(false);
        //100s
        mUtil.notifyRead(MediaPlayerUtil::readEvent_Loop);

        if (!mEof) {
            //demuxer read
            int64_t read_start_time = af_gettime_relative();
            int timeout = 10000;

            if (mSet.bDisableBufferManager) {
                timeout = 5000;
            }

            mem_info info{};
            int checkStep = 0;

            while (true) {
                // once buffer is full, we will try to read again if buffer consume more then BufferGap
                if (mBufferIsFull) {
                    static const int BufferGap = 1000 * 1000;

                    if ((mSet.maxBufferDuration > 2 * BufferGap)
                            && (cur_buffer_duration > mSet.maxBufferDuration - BufferGap)) {
                        break;
                    }
                }

                if (cur_buffer_duration > mSet.maxBufferDuration) {
                    mBufferIsFull = true;
                    break;
                }

                mBufferIsFull = false;

                if ((0 >= checkStep--)
                        && (cur_buffer_duration > 1000 * 1000)
                        && (AFGetSystemMemInfo(&info) >= 0)) {
                    //AF_LOGD("system_availableram is %" PRIu64 "",info.system_availableram);
                    if (info.system_availableram > 2 * mSet.lowMemSize) {
                        checkStep = (int) (info.system_availableram / (5 * 1024 * 1024));
                    } else if (info.system_availableram < mSet.lowMemSize) {
                        AF_LOGW("low memery...");

                        if (!mLowMem) {
                            mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_SYSTEM_LOW_MEMORY, "App Low memory");
                        }

                        mLowMem = true;

                        if (mSet.highLevelBufferDuration > 800 * 1000) {
                            mSet.highLevelBufferDuration = 800 * 1000;
                        }

                        if (mSet.startBufferDuration > 800 * 1000) {
                            mSet.startBufferDuration = 800 * 1000;
                        }

                        break;
                    } else {
                        checkStep = 5;
                        mLowMem = false;
                    }
                }

                int ret = ReadPacket();

                if (ret == -EAGAIN) {
                    if (0 == mDuration) {
                        mRemainLiveSegment = mDemuxerService->GetRemainSegmentCount(mCurrentVideoIndex);
                    }

                    mUtil.notifyRead(MediaPlayerUtil::readEvent_Again);
                    break;
                } else if (ret == 0) {
                    AF_LOGE("Player ReadPacket EOF");

                    if (!mEof) {
                        mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_DEMUXER_EOF, "Demuxer End of File");
                    }

                    mEof = true;
                    break;
                } else if (ret == FRAMEWORK_ERR_EXIT) {
                    AF_LOGE("Player ReadPacket error 0x%04x :%s\n", -ret, framework_err2_string(ret));
                    break;
                } else if (ret == FRAMEWORK_ERR_FORMAT_NOT_SUPPORT) {
                    AF_LOGW("read error %s\n", framework_err2_string(ret));
                    break;
                } else if (ret < 0) {
                    if (!mBufferingFlag) {
                        //AF_LOGI("Player ReadPacket ret < 0 with data");
                    } else {
                        AF_LOGE("Player ReadPacket error 0x%04x :%s\n", -ret, framework_err2_string(ret));

                        if (ret != FRAMEWORK_ERR_EXIT && !mCanceled) {
                            NotifyError(ret);
                        }
                    }

                    break;
                }

                //AF_LOGI("Player ReadPacket have data");
                if (0 >= mFirstReadPacketSucMS) {
                    mFirstReadPacketSucMS = af_getsteady_ms();
                }

                if (af_gettime_relative() - read_start_time > timeout) {
                    AF_LOGD("Player ReadPacket time out\n");
                    mUtil.notifyRead(MediaPlayerUtil::readEvent_timeOut);
//                    mMsgProcessTime = 0;
                    break;
                }

                cur_buffer_duration = getPlayerBufferDuration(false);
//                if(getPlayerBufferDuration(true) > mSet.maxBufferDuration * 2){
//                    AF_LOGE("buffer stuffed\n");
//                    mPNotifier->NotifyError(MEDIA_PLAYER_ERROR_BUFFER_STUFFED,"buffer stuffed");
//                    break;
//                }
            }
        }
    }

    void SuperMediaPlayer::OnDemuxerCallback(const std::string &key, const std::string &value)
    {
    }

    bool SuperMediaPlayer::DoCheckBufferPass()
    {
        int64_t cur_buffer_duration = getPlayerBufferDuration(false);
        int64_t HighBufferDur = mSet.highLevelBufferDuration;

        if (mSet.bDisableBufferManager) {
            HighBufferDur = 10 * 1000;
        }

        if (mFirstBufferFlag && !mEof) {
            if (!mSet.bDisableBufferManager) {
                HighBufferDur = mSet.startBufferDuration;
            }

            //clean late audio data
            if (cur_buffer_duration > HighBufferDur && HAVE_VIDEO && HAVE_AUDIO) {
                if (mSoughtVideoPos > 0) {
                    AF_LOGW("clean late audio data");
                    mBufferController.ClearPacketBeforeTimePos(BUFFER_TYPE_AUDIO, mSoughtVideoPos);
                    int64_t pts = mBufferController.GetPacketPts(BUFFER_TYPE_AUDIO);

                    if (mRemovedFirstAudioPts == INT64_MIN) {
                        mRemovedFirstAudioPts = pts;

                        if (mFirstAudioPts == INT64_MIN) {
                            mFirstAudioPts = pts - mFirstSeekStartTime;
                        }
                    }

                    cur_buffer_duration = getPlayerBufferDuration(false);

                    if (cur_buffer_duration < HighBufferDur) {
                        return false;
                    }
                }
            }
        }

        if (mPlayStatus == PLAYER_PREPARING) {
            if ((cur_buffer_duration >= HighBufferDur && (!HAVE_VIDEO || videoDecoderFull || APP_BACKGROUND == mAppStatus)) || (mEof)) {
                //open stream failed
                if (mEof && getPlayerBufferDuration(true) <= 0) {
                    ChangePlayerStatus(PLAYER_ERROR);
                    mPNotifier->NotifyError(MEDIA_PLAYER_ERROR_DEMUXER_OPENSTREAM, "open stream failed");
                } else {
                    ChangePlayerStatus(PLAYER_PREPARED);
                    AF_LOGD("PLAYER_PREPARED");
                    AF_LOGD("prepare use %lld ms\n",
                            (af_gettime_relative() - mPrepareStartTime) / 1000);
                    notifyPreparedCallback();

                    if (mFirstBufferFlag) {
                        mFirstBufferFlag = false;
                    }
                }
            }
        }

        //check buffering empty
        if (!mEof && cur_buffer_duration <= 0 && !mBufferingFlag
                && (mPlayStatus == PLAYER_PLAYING || mPlayStatus == PLAYER_PAUSED)
                && !mSet.bDisableBufferManager) {
            mBufferingFlag = true;
            mPNotifier->NotifyLoading(loading_event_start, 0);
            AF_LOGD("loading start");
            mLoadingProcess = 0;
            mTimeoutStartTime = INT64_MIN;
            mMasterClock.pause();
            return false;
        }

        //AF_LOGD("current duration is %lld,video duration is %lld,audio duration is %lld", cur_buffer_duration
        //	,mBufferController.GetPacketDuration(BUFFER_TYPE_VIDEO), mBufferController.GetPacketDuration(BUFFER_TYPE_AUDIO));
        while (IS_REAL_TIME_STREAM && mSet.RTMaxDelayTime > 0) {
            if (!HAVE_AUDIO) {
                int64_t maxBufferDuration = getPlayerBufferDuration(true);

                if (maxBufferDuration > mSet.RTMaxDelayTime + 1000 * 1000 * 5) {
                    int64_t lastKeyPos = mBufferController.GetPacketLastKeyTimePos(BUFFER_TYPE_VIDEO);
                    mBufferController.ClearPacketBeforeTimePos(BUFFER_TYPE_VIDEO, lastKeyPos);
                    break;
                }

                LiveCatchUp(maxBufferDuration);
                break;
            }

            int64_t maxBufferDuration = getPlayerBufferDuration(true);

            if (maxBufferDuration > mSet.RTMaxDelayTime + 1000 * 1000 * 5) {
                //drop frame
                int64_t lastVideoPos = mBufferController.GetPacketLastTimePos(BUFFER_TYPE_VIDEO);
                int64_t lastAudioPos = mBufferController.GetPacketLastTimePos(BUFFER_TYPE_AUDIO);
                int64_t lastAudioPts = mBufferController.GetPacketLastPTS(BUFFER_TYPE_AUDIO);
                int64_t lastPos;

                if (lastVideoPos == INT64_MIN) {
                    lastPos = lastAudioPos;
                } else if (lastAudioPos == INT64_MIN) {
                    lastPos = lastVideoPos;
                } else {
                    lastPos = lastAudioPos < lastVideoPos ? lastAudioPos : lastVideoPos;
                }

                lastPos -= min(mSet.RTMaxDelayTime, 500 * 1000);
                int64_t lastVideoKeyTimePos = mBufferController.GetKeyTimePositionBefore(BUFFER_TYPE_VIDEO, lastPos);
                if (lastVideoKeyTimePos != INT64_MIN) {
                    AF_LOGD("drop left lastPts %lld, lastVideoKeyPts %lld", lastPos, lastVideoKeyTimePos);
                    ProcessSetSpeed(1.0);
                    int64_t dropVideoCount = mBufferController.ClearPacketBeforeTimePos(BUFFER_TYPE_VIDEO, lastVideoKeyTimePos);
                    int64_t dropAudioCount = mBufferController.ClearPacketBeforeTimePos(BUFFER_TYPE_AUDIO, lastVideoKeyTimePos);

                    if (dropVideoCount > 0) {
                        FlushVideoPath();
                        AF_LOGD("drop left video duration is %lld,left video size is %d",
                                mBufferController.GetPacketDuration(BUFFER_TYPE_VIDEO),
                                mBufferController.GetPacketSize(BUFFER_TYPE_VIDEO));
                    }

                    if (dropAudioCount > 0) {
                        FlushAudioPath();
                        AF_LOGD("drop left aduio duration is %lld,left aduio size is %d",
                                mBufferController.GetPacketDuration(BUFFER_TYPE_AUDIO),
                                mBufferController.GetPacketSize(BUFFER_TYPE_AUDIO));
                        mMasterClock.setTime(lastAudioPts);
                    }
                }
            }

            int64_t lastAudio = mBufferController.GetPacketLastPTS(BUFFER_TYPE_AUDIO);

            if ((lastAudio != INT64_MIN) && (mPlayedAudioPts != INT64_MIN)) {
                int64_t playTime = getAudioPlayTimeStamp();

                if (INT64_MIN == playTime) {
                    playTime = mPlayedAudioPts;
                }

                int64_t delayTime = lastAudio - playTime;
                static int64_t lastT = af_gettime_ms();

                if (af_gettime_ms() - lastT > 1000) {
                    lastT = af_gettime_ms();
                    AF_LOGD("lastAudio:%lld mPlayedAudioPts:%lld, delayTime:%lld", lastAudio, mPlayedAudioPts, delayTime);
                }

                LiveCatchUp(delayTime);
            }

            break;
        }

        //check buffering status
        if ((mBufferingFlag || mFirstBufferFlag) && !mSet.bDisableBufferManager) {
            if ((cur_buffer_duration > HighBufferDur && (!HAVE_VIDEO || videoDecoderFull || APP_BACKGROUND == mAppStatus)) || mEof) {
                // if still in seek, wait for seek status be changed.
                if (!mSeekFlag) {
                    if (mBufferingFlag) {
                        mLoadingProcess = -1;
                        mPNotifier->NotifyLoading(loading_event_end, 0);
                        AF_LOGD("loading end");

                        if (mPlayStatus == PLAYER_PLAYING) {
                            mMasterClock.start();
                        }
                    }

                    mTimeoutStartTime = INT64_MIN;
                    mFirstBufferFlag = false;
                    mBufferingFlag = false;
                }
            } else if (cur_buffer_duration >= 0) {
                // TODO: Notify when change
                int prg = MIN(static_cast<int>(cur_buffer_duration * 100 / HighBufferDur), 100);

                if (prg > mLoadingProcess && mBufferingFlag) {
                    mLoadingProcess = prg;
                    mPNotifier->NotifyLoading(loading_event_progress, prg);
                    mTimeoutStartTime = INT64_MIN;
                } else if (mLoadingProcess == prg) {//15seconds loading progress not changed
                    int64_t curTime = af_gettime_relative() / 1000;

                    if ((mTimeoutStartTime == INT64_MIN)
                            || (PLAYER_PLAYING != mPlayStatus)) {
                        mTimeoutStartTime = curTime;
                    } else if (curTime - mTimeoutStartTime >= mSet.timeout_ms) {
                        //TODO: demuxer still read crazy
                        if (mLowMem) {
                            mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_SYSTEM_LOW_MEMORY, "App Low memory");
                            // if it's a network error or waiting for network to retry, don't notify error
                        } else if (mSourceListener.isConnected() && !mSourceListener.isPending()) {
                            //    ChangePlayerStatus(PLAYER_ERROR);
                            //   notifyErrorCallback(MEDIA_PLAYER_ERROR_LOADING_TIMEOUT, "Loading timeout");
                        }

                        mTimeoutStartTime = curTime;
                    }
                }
            }
        }

        return true;
    }

    void SuperMediaPlayer::LiveCatchUp(int64_t delayTime)
    {
        int recoverGap = 50 * 1000;

        if (mSet.RTMaxDelayTime >= 1000 * 1000) {
            recoverGap = 500 * 1000;
        } else if (mSet.RTMaxDelayTime >= 200 * 1000) {
            recoverGap = 100 * 1000;
        }

        if ((delayTime > mSet.RTMaxDelayTime) && (150 * 1000 < delayTime)) {
            ProcessSetSpeed(1.2);
        } else if ((delayTime < mSet.RTMaxDelayTime - recoverGap) || (100 * 1000 > delayTime)) {
            ProcessSetSpeed(1.0);
        }
    }

    void SuperMediaPlayer::notifyPreparedCallback()
    {
        if (waitingForStart && mSet.bLooping) {
            //when loop play don`t send prepare msg.
        } else {
            mPNotifier->NotifyPrepared();
        }

        if (waitingForStart || mAutoPlay) {
            Start();
            waitingForStart = false;
        }

        if (mAutoPlay) {
            mPNotifier->NotifyAutoPlayStart();
        }
    }

    void SuperMediaPlayer::doRender()
    {
        bool rendered = false;

        if (mFirstBufferFlag && mPlayStatus != PLAYER_PREPARING) {
            if (HAVE_VIDEO && mAppStatus != APP_BACKGROUND) {
                rendered = RenderVideo(true);

                if (rendered) {
                    AF_LOGD("TIMEPOS RenderVideo :%lld", mPlayedVideoPts / 1000);
                    mMasterClock.setTime(mPlayedVideoPts);
                    NotifyPosition(getCurrentPosition() / 1000);

                    // seek preview can't render audio,but set the audio clock to here pts
                    if (HAVE_AUDIO && (mAudioTime.startTime <= 0)) {
                        if (!mAudioFrameQue.empty()) {
                            mAudioTime.startTime = mAudioFrameQue.front()->getInfo().pts;
                        } else {
                            mAudioTime.startTime = mPlayedVideoPts;
                        }
                    }
                }
            } else {   // audio only
                if (!mAudioFrameQue.empty()) {
                    int64_t audioPTS = mAudioFrameQue.front()->getInfo().pts;
                    NotifyPosition(audioPTS / 1000);
                    rendered = true;
                    mAudioTime.startTime = audioPTS;
                }
            }
        }

        if (mPlayStatus == PLAYER_PLAYING) {
            if (!mBufferingFlag) {
                rendered |= render();
            }
        }

        if (rendered) {
            mFirstBufferFlag = false;

            //may audio already played over
            if (mEof && mAudioFrameQue.empty() && mBufferController.GetPacketSize(BUFFER_TYPE_AUDIO) == 0) {
                mMasterClock.setReferenceClock(nullptr, nullptr);
            }

            if (mSeekFlag) {
                mSeekFlag = false;

                if (!mMessageControl.findMsgByType(MSG_SEEKTO)) {
                    ResetSeekStatus();
                    mPNotifier->NotifySeekEnd(mSeekInCache);
                    mSeekInCache = false;

                    //update position when seek end. in case of when paused.
                    if (mPlayStatus == PLAYER_PAUSED) {
                        NotifyPosition(getCurrentPosition() / 1000);
                    }
                }
            }
        }
    }

    void SuperMediaPlayer::doDeCode()
    {
        //get video packet to decode
        if (HAVE_VIDEO && !videoDecoderEOS && mVideoDecoder) {
            int max_cache_size = VIDEO_PICTURE_MAX_CACHE_SIZE;

            if (mPictureCacheType == picture_cache_type_cannot) {
                max_cache_size = 1;
            }

            unsigned long videoFrameSize = mVideoFrameQue.size();

            if (videoFrameSize < max_cache_size) {
                int64_t startDecodeTime = af_getsteady_ms();
                int64_t videoEarlyUs = 0;

                do {
                    // if still in seeking, don't send data to decoder in background
                    // due to the playback position could be changed later.
                    if ((APP_BACKGROUND == mAppStatus) && isSeeking()) {
                        break;
                    }

                    if (mVideoPacket == nullptr) {
                        mVideoPacket = mBufferController.getPacket(BUFFER_TYPE_VIDEO);
                    }

                    videoEarlyUs = mVideoPacket ? mVideoPacket->getInfo().dts - mMasterClock.GetTime() : 0;

                    // don't send too much data when in background
                    if (mVideoPacket && APP_BACKGROUND == mAppStatus && videoEarlyUs > 0) {
                        break;
                    }

                    FillVideoFrame();

                    if (nullptr == mVideoPacket && !mEof) {
                        break;
                    }

                    if (mVideoPacket && !HAVE_AUDIO) {
                        if (mVideoPacket->getInfo().timePosition >= 0) {
                            mCurrentPos = mVideoPacket->getInfo().timePosition;
                            printTimePosition(mCurrentPos);
                        }
                    }

                    int ret = DecodeVideoPacket(mVideoPacket);

                    if (ret & STATUS_RETRY_IN) {
                        break;
                    }

                    if (af_getsteady_ms() - startDecodeTime > 50) {
                        break;
                    }
                } while ((mSeekNeedCatch || dropLateVideoFrames) && (videoEarlyUs < 200 * 1000));
            }
        }

        //get audio packet to decode
        if (HAVE_AUDIO && !audioDecoderEOS && mAudioDecoder) {
            for (int i = 0; i < 2; ++i) {
                unsigned long frameCount = mAudioFrameQue.size();

                if (frameCount < 4) {
                    if (mAudioPacket == nullptr) {
                        mAudioPacket = mBufferController.getPacket(BUFFER_TYPE_AUDIO);
                    }

                    if (mAudioPacket) {
                        int64_t timePosition = mAudioPacket->getInfo().timePosition;
                        DecodeAudio(mAudioPacket);

                        if (mAudioPacket == nullptr && timePosition >= 0) {
                            mCurrentPos = timePosition;
                            //printTimePosition(mCurrentPos);
                        }
                    } else if (mEof) {
                        unique_ptr<IAFPacket> packet{};
                        DecodeAudio(packet);
                    }
                } else {
                    break;
                }
            }
        }
    }

    void SuperMediaPlayer::checkEOS()
    {
        if (!mEof || PLAYER_COMPLETION == mPlayStatus) {
            return;
        }

        //in case of ONLY AUDIO stream.
        if ((HAVE_VIDEO && !videoDecoderEOS && (APP_BACKGROUND != mAppStatus))
                || (HAVE_AUDIO && !audioDecoderEOS)) {
            return;
        }

        int packetSize = mBufferController.GetPacketSize(BUFFER_TYPE_VIDEO);
        int frameSize = static_cast<int>(mAudioFrameQue.size());

        if ((APP_BACKGROUND != mAppStatus)) {
            frameSize += mVideoFrameQue.size();
            packetSize += mBufferController.GetPacketSize(BUFFER_TYPE_AUDIO);
        }

        if (frameSize > 0 || packetSize > 0) {
            AF_TRACE;
            return;
        }

        if (mAudioRender != nullptr) {
            uint64_t audioQueDuration = mAudioRender->getQueDuration();

            if (audioQueDuration != 0) {
                AF_TRACE;
//work around: xiaomi 5X 7.1.2 audioTrack getPosition always is 0 when seek to end
                int64_t now = af_getsteady_ms();

                if (mCheckAudioQueEOSTime == INT64_MIN || mAudioQueDuration != audioQueDuration) {
                    mCheckAudioQueEOSTime = now;
                    mAudioQueDuration = audioQueDuration;
                }

                if ((now - mCheckAudioQueEOSTime) * 1000 <= audioQueDuration) {
                    return;
                }
            }
        }

        NotifyPosition(mDuration / 1000);

        if (mSet.bLooping && mDuration > 0) {
            mSeekPos = 0; //19644161: need reset seek position
            ProcessSeekToMsg(0, false);
            mPNotifier->NotifyLoopStart();
            NotifyPosition(0);
        } else {
            if (mPlayStatus != PLAYER_COMPLETION) {
                if (APP_BACKGROUND == mAppStatus) {
                    FlushVideoPath();
                    mBufferController.ClearPacket(BUFFER_TYPE_VIDEO);
                }

                mPNotifier->NotifyCompletion();
                ChangePlayerStatus(PLAYER_COMPLETION);
                mUtil.reset();
            }
        }
    }

    int SuperMediaPlayer::DecodeVideoPacket(unique_ptr<IAFPacket> &pVideoPacket)
    {
        int ret = 0;

        if (videoDecoderEOS) {
            return ret;
        }

        int64_t pos = getCurrentPosition();

        if (pVideoPacket != nullptr) {
            // for cache video, or seeking accurate, check whether drop output frame
            if (mSeekNeedCatch || dropLateVideoFrames) {
                int64_t checkPos = mSeekNeedCatch ? mSeekPos.load() : pos;

                // only decode and don't need output to render if too old
                if ((pVideoPacket->getInfo().timePosition < checkPos)
                        && (pVideoPacket->getInfo().timePosition < mDuration - 200 * 1000)) {
                    pVideoPacket->setDiscard(true);
                }
            }

            ret = mVideoDecoder->send_packet(pVideoPacket, 0);

            // don't need pop if need retry later
            if (!(ret & STATUS_RETRY_IN)) {
                //  mBufferController.PopFrontPacket(BUFFER_TYPE_VIDEO);
                assert(pVideoPacket == nullptr);
            }
        } else if (mEof) {
            mVideoDecoder->setEOF();
            mVideoDecoder->send_packet(pVideoPacket, 0);
            ret = 0;
        }

        if (ret > 0) {
            bool haveError = false;

            if (ret & STATUS_HAVE_ERROR) {
                if (mVideoDecoder->get_error_frame_no() > MAX_DECODE_ERROR_FRAME) {
                    haveError = true;
                }
            }

//            if (ret & STATUS_CREATE_FAIL) {
//                haveError = true;
//            }

            if (haveError) {
                ChangePlayerStatus(PLAYER_ERROR);
                mPNotifier->NotifyError(MEDIA_PLAYER_ERROR_DECODE_VIDEO, "video decode error");
            }
        }

        return ret;
    }

    int SuperMediaPlayer::FillVideoFrame()
    {
        int64_t pos = getCurrentPosition();
        unique_ptr<IAFFrame> pFrame{};
        int ret = mVideoDecoder->getFrame(pFrame, 0);

        if (ret == STATUS_EOS) {
            videoDecoderEOS = true;
        }

        if (pFrame != nullptr) {
            mVideoDecoder->clean_error();
            int64_t pts = pFrame->getInfo().pts;

            if (mSeekFlag && mSeekNeedCatch) {
                mSeekNeedCatch = false;
            }

            Stream_meta *meta = (Stream_meta *) (mCurrentVideoMeta.get());

            if (meta->displayWidth > 0 && meta->displayHeight > 0) {
                pFrame->getInfo().video.dar = 1.0 * meta->displayWidth / meta->displayHeight;
            } else {
                //if not get displaywidth/height , set dar with width/height
                pFrame->getInfo().video.dar = 1.0 * pFrame->getInfo().video.width / pFrame->getInfo().video.height;
            }

            mDemuxerService->SetOption("FRAME_DECODED", pts);
//            AF_LOGI("DecodeVideoPacket p_dec_delay frame :%lld pos:%lld, mPlayedAudioPts:%lld, posdiff:%lld audiodiff:%lld videodiff:%lld",
//                    pFrame->GetPts()/1000, pos/1000, mPlayedAudioPts/1000, (pos - pFrame->GetPts())/1000,
//                    (mLastInputAudio - mPlayedAudioPts)/1000, (mLastInputVideo - pFrame->GetPts())/1000);
            mVideoFrameQue.push(move(pFrame));
            videoDecoderFull = true;
        }

        return ret;
    }

    bool SuperMediaPlayer::render()
    {
        //send to audio render
        bool audioRendered = false;
        bool videoRendered = false;

        if ((mCurrentAudioIndex >= 0) && !mSeekNeedCatch) {
            int ret = RenderAudio();

            if (RENDER_NONE != ret) {
                audioRendered = true;

                // if send all audio data, try to render again to send more data.
                if (RENDER_FULL == ret) {
                    RenderAudio();
                }
            }
        }

        if (HAVE_VIDEO) {
            videoRendered = RenderVideo(false);
        }

        if (HAVE_VIDEO && HAVE_AUDIO) {
            // TODO: do it in reset()
            if (!PTS_REVERTING) {
                if (videoRendered && audioRendered) {
                    mVideoPtsRevert = mAudioPtsRevert = false;
                }
            } else {
                AF_LOGW("PTS_REVERTING audio pts is %lld ,video pts is %lld\n", mPlayedAudioPts, mPlayedVideoPts);
            }
        } else {
            mVideoPtsRevert = mAudioPtsRevert = false;
        }

        if ((HAVE_SUBTITLE || mSubPlayer) && !mSeekFlag) {
            RenderSubtitle(mCurVideoPts);
        }

        return audioRendered || videoRendered;
    }

    RENDER_RESULT SuperMediaPlayer::RenderAudio()
    {
        RENDER_RESULT ret = RENDER_NONE;
        int64_t pts = INT64_MIN;
        int64_t duration = INT64_MIN;
        int render_ret;
//#define DUMP_PCM
#ifdef DUMP_PCM
        static int fd = open("out.pcm", O_CREAT | O_RDWR, 0666);
#endif

        if (mAudioFrameQue.empty()) {
            if (audioDecoderEOS && mAudioRender->getQueDuration() == 0) {
                mMasterClock.setReferenceClock(nullptr, nullptr);
            }

            return ret;
        }

        pts = mAudioFrameQue.front()->getInfo().pts;

        if (pts == INT64_MIN) {
            mAudioFrameQue.pop_front();
            return ret;
        }

        auto *avafFrame = dynamic_cast<AVAFFrame *>(mAudioFrameQue.front().get());

        if (avafFrame) {
            duration = getPCMFrameDuration(avafFrame->ToAVFrame());
        }

        if (mFrameCb) {
            mFrameCb(mFrameCbUserData, avafFrame);
        }

        render_ret = mAudioRender->renderFrame(mAudioFrameQue.front(), 0);

        if (render_ret == IAudioRender::FORMAT_NOT_SUPPORT) {
            if (mAudioRender->getQueDuration() == 0) {
                std::lock_guard<std::mutex> uMutex(mCreateMutex);
                mAudioRender = nullptr;
                mAudioTime.startTime = mAudioFrameQue.front()->getInfo().pts;
                mAudioTime.deltaTimeTmp = 0;
                mAudioTime.deltaTime = 0;
                mLastAudioFrameDuration = -1;
                setUpAudioRender(mAudioFrameQue.front()->getInfo().audio);
                if (mBRendingStart) {
                    mAudioRender->pause(false);
                }
                mAudioRender->renderFrame(mAudioFrameQue.front(), 0);
            }
        }

        if (mAudioFrameQue.front() == nullptr) {
            mAudioFrameQue.pop_front();
            ret = RENDER_FULL;
        } else {
            return ret;
        }

#ifdef DUMP_PCM
        write(fd, buffer + mWriteAudioLen, len);
#endif

        if (!HAVE_VIDEO) {
            checkFirstRender();
        }

        if (mPlayedAudioPts == INT64_MIN) {
            mAudioTime.startTime = pts;
            mAudioTime.deltaTime = 0;
            mAudioTime.deltaTimeTmp = 0;
            mMasterClock.setReferenceClock(getAudioPlayTimeStampCB, this);
        } else {
            if (mLastAudioFrameDuration > 0) {
                int64_t offset = pts - (mPlayedAudioPts + mLastAudioFrameDuration);

                if (llabs(offset) > 200000) {
                    AF_LOGW("offset is %lld,pts is %lld", offset, pts);
                }

                mAudioTime.deltaTimeTmp += offset;

                if (llabs(mAudioTime.deltaTimeTmp) > 100000) {
                    AF_LOGW("correct audio and master clock offset is %lld, frameDuration :%lld", mAudioTime.deltaTimeTmp,
                            mLastAudioFrameDuration);
                    mAudioTime.deltaTime += mAudioTime.deltaTimeTmp;
                    mAudioTime.deltaTimeTmp = 0;
                }
            }
        }

        if (!mAudioPtsRevert) {
            mAudioPtsRevert = mPlayedAudioPts != INT64_MIN && pts < mPlayedAudioPts - PTS_DISCONTINUE_DELTA;

            if (mAudioPtsRevert) {
                AF_LOGI("PTS_REVERTING audio start\n");
            }
        }

        mPlayedAudioPts = pts;
        mLastAudioFrameDuration = duration;

        if (mAudioChangedFirstPts == pts && !mMixMode) {
            StreamInfo *info = GetCurrentStreamInfo(ST_TYPE_AUDIO);
            mPNotifier->NotifyStreamChanged(info, ST_TYPE_AUDIO);
            AF_LOGD("audio changed\n");
            mAudioChangedFirstPts = INT64_MIN;
        }

        return ret;
    }

    bool SuperMediaPlayer::RenderVideo(bool force_render)
    {
        //send to video render
        if (mVideoFrameQue.empty()) {
            return false;
        }

        unique_ptr<IAFFrame> &videoFrame = mVideoFrameQue.front();

        if (videoFrame == nullptr) {
            return false;
        }

        int64_t videoPts = videoFrame->getInfo().pts;

        // work around for huaweiP20 pro hardware decode get pts = INT64_MIN when change resolution.
        if (videoPts == INT64_MIN && videoPts < mPlayedVideoPts) {
            videoPts = mPlayedVideoPts + 1;
        }

        int frameWidth = videoFrame->getInfo().video.width;
        int frameHeight = videoFrame->getInfo().video.height;
        int frameRotate = videoFrame->getInfo().video.rotate;

        if (!mVideoPtsRevert) {
            mVideoPtsRevert = mPlayedVideoPts != INT64_MIN && videoPts < mPlayedVideoPts - PTS_DISCONTINUE_DELTA;

            if (mVideoPtsRevert) {
                AF_LOGI("PTS_REVERTING video start\n");
            }
        }

        // audio pts first revert to small, force render the old video frame
        if (PTS_REVERTING && mAudioPtsRevert && videoPts - PTS_DISCONTINUE_DELTA > mPlayedAudioPts) {
            AF_LOGI("PTS_REVERTING force render the old video frame");
            force_render = true;
            // video pts first revert to small,the new video data wait audio pts to revert
        } else if (PTS_REVERTING && mVideoPtsRevert && videoPts + PTS_DISCONTINUE_DELTA < mPlayedAudioPts) {
            AF_LOGI("PTS_REVERTING wait audio to revert");
            return false;
        }

        int64_t masterPlayedTime = mMasterClock.GetTime();
        int64_t videoLateUs = masterPlayedTime - videoPts;// if videoLateUs > 0, video is late

        /*
         *  if stc is free, video rectify it
         */
        if ((llabs(videoLateUs) > 1000 * 1000)
                || (llabs(videoLateUs) > mSet.maxBufferDuration)) {
            // don't have master, or master not in valid status
            if (!mMasterClock.haveMaster() || !mMasterClock.isMasterValid()) {
                mMasterClock.setTime(videoPts);
                masterPlayedTime = videoPts;
            }
        }

        /* video early more than 10ms, don't (render||drop) it, deal it next time
         * if the pts is not continue drop it

         -------------------early----------------------------|------------------------late--------------

         --------------|-------------------------------|-----|-------------|--------------------------------------->
             drop      10s       render next time     10ms   0            500ms     drop

                                                       |--------render-----|


         */
        bool render = force_render;

        if (!force_render) {
            if (videoLateUs < -10 * 1000 && videoLateUs > -PTS_DISCONTINUE_DELTA) {
                return false;
            }

            // To avoid video frame packet overload when video decoder can't catch up.
            if (videoLateUs >= 500 * 1000 && !PTS_REVERTING) {
                int64_t lastVideoKeyPts = mBufferController.GetKeyPTSBefore(BUFFER_TYPE_VIDEO, masterPlayedTime);

                if (lastVideoKeyPts != INT64_MIN) {
                    int64_t dropVideoCount = mBufferController.ClearPacketBeforePts(BUFFER_TYPE_VIDEO, lastVideoKeyPts);

                    if (dropVideoCount > 0) {
                        FlushVideoPath();
                        AF_LOGD("videolaterUs is %lld,drop video count is %d", videoLateUs, dropVideoCount);
                        return false;
                    }
                }
            }

            if (dropLateVideoFrames) {
                if (videoLateUs > 10 * 1000) {
                    render = false;
                } else {
                    dropLateVideoFrames = false;
                }
            } else if (videoLateUs < 500 * 1000) { // video early litter than 10ms or late litter than 500ms,render it
                render = true;
            }
            // Try to render if already haven't render more then 60ms
            else if (INT64_MIN == mPlayedVideoPts || (videoPts - mPlayedVideoPts) > 60 * 1000) {
                render = true;
            }
        }

        if (mFrameCb) {
            mFrameCb(mFrameCbUserData, videoFrame.get());
        }

        if (render) {
            SendVideoFrameToRender(move(videoFrame));

            if (frameWidth != mVideoWidth || frameHeight != mVideoHeight) {
                mVideoWidth = frameWidth;
                mVideoHeight = frameHeight;
                mVideoRotation = frameRotate;
                mPNotifier->NotifyVideoSizeChanged(mVideoWidth, mVideoHeight);
            }

            if (!HAVE_AUDIO) {
                if (mPlayedVideoPts == INT64_MIN) {
                    mMasterClock.setTime(videoPts);
                    mMasterClock.setReferenceClock(mClockRef, mCRArg);
                }
            }
        } else {
            AF_LOGW("drop frame,master played time is %lld,video pts is %lld\n", masterPlayedTime, videoPts);
            VideoRenderCallback(this, videoPts, nullptr);
            videoFrame->setDiscard(true);
        }

        mPlayedVideoPts = videoPts;
        mVideoFrameQue.pop();
        return render;
    }

    void SuperMediaPlayer::RenderSubtitle(int64_t pts)
    {
        if (mSubPlayer && mSubPlayer->isActive()) {
            mSubPlayer->update(pts);
            return;
        }

        auto iter = mSubtitleShowedQueue.begin();

        while (iter != mSubtitleShowedQueue.end()) {
            if ((*iter).get()) {
                if (((*iter)->getInfo().pts + (*iter)->getInfo().duration) <= pts) {
                    mPNotifier->NotifySubtitleEvent(subTitle_event_hide, (*iter).release(), 0, nullptr);
                    iter = mSubtitleShowedQueue.erase(iter);
                    continue;
                }
            }

            iter++;
        }

        int64_t subTitlePts = mBufferController.GetPacketPts(BUFFER_TYPE_SUBTITLE);

        if (subTitlePts == INT64_MIN || subTitlePts > pts) {
            return;
        }

        unique_ptr<IAFPacket> pFrame = mBufferController.getPacket(BUFFER_TYPE_SUBTITLE);

        if (pFrame == nullptr) {
            return;
        }

        bool changed = false;

        if (pFrame->getInfo().pts + pFrame->getInfo().duration < pts) {
            AF_LOGD("read subtitle pts is long before pts is %lld,subtitle pts is %lld", pts, subTitlePts);
            mBufferController.ClearPacketBeforePts(BUFFER_TYPE_SUBTITLE, subTitlePts);

            if (subTitlePts <= mSubtitleChangedFirstPts && pts > mSubtitleChangedFirstPts) {
                changed = true;
            }
        } else {
            if (subTitlePts == mSubtitleChangedFirstPts) {
                changed = true;
            }

            AF_LOGD("read subtitle pts show pts is %lld,subtitle pts is %lld", pts, subTitlePts);
            pFrame->getInfo().dts = mSubtitleShowIndex++;
//          pFrame->pBuffer[pFrame->size] = 0;
            mPNotifier->NotifySubtitleEvent(subTitle_event_show, pFrame.get(), 0, nullptr);
            mSubtitleShowedQueue.push_back(move(pFrame));
        }

        if (changed) {
            AF_LOGD("subtitle changed");
            StreamInfo *info = GetCurrentStreamInfo(ST_TYPE_SUB);
            mPNotifier->NotifyStreamChanged(info, ST_TYPE_SUB);
            mSubtitleChangedFirstPts = INT64_MIN;
        }
    }

    void SuperMediaPlayer::OnTimer(int64_t curTime)
    {
        if (mPlayedAudioPts != INT64_MIN || mPlayedVideoPts != INT64_MIN) {
            updateLoopGap();

            /*
             * if have seek not completed,DO NOT update the position,it will lead process bar
             * jumping
             */
            if ((mPlayStatus == PLAYER_PLAYING) &&
                    !isSeeking()) {
                int64_t pos = getCurrentPosition() / 1000;
                //AF_LOGD("TIMEPOS OnTimer :%lld", pos);
                NotifyPosition(pos);
            }

            PostBufferPositionMsg();
        }
    }

    void SuperMediaPlayer::SendVideoFrameToRender(unique_ptr<IAFFrame> frame, bool valid)
    {
//#ifdef WIN32
//
//        if (mVideoDecodeType == AlivcVideoCodecHardware) {
//            m_dxva2Render.RenderFrame(frame->getFrame()->GetData());
//            //delete pFrame;
//            return;
//        }
//
//#endif
        if (mVideoRender) {
            int ret = mVideoRender->renderFrame(frame);

            if (ret < 0) {
                AF_LOGE("renderFrame error \n");
                // for windows init failed, which may need change render type in future.
                mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_VIDEO_RENDER_INIT_ERROR, "init video render failed");
            }
        }
    }

    int SuperMediaPlayer::DecodeAudio(unique_ptr<IAFPacket> &pPacket)
    {
        if (mAudioDecoder == nullptr) {
            return -EINVAL;
        }

        if (audioDecoderEOS) {
            return 0;
        }

        unique_ptr<IAFFrame> frame{};
        int ret;

        do {
            ret = mAudioDecoder->getFrame(frame, 0);

            if (ret == STATUS_EOS) {
                audioDecoderEOS = true;
                break;
            }

            if (frame != nullptr) {
                if (frame->getInfo().pts == INT64_MIN) {
                    // TODO: why mAudioFrameQue.back()->getInfo().pts is INT64_MIN
                    if (!mAudioFrameQue.empty() && mAudioFrameQue.back()->getInfo().pts != INT64_MIN) {
                        double duration = ((double) frame->getInfo().audio.nb_samples) / frame->getInfo().audio.sample_rate;
                        frame->getInfo().pts = mAudioFrameQue.back()->getInfo().pts + duration * 1000000;
                    } else {
                        //                       assert(0);
                    }
                }

                mAudioFrameQue.push_back(move(frame));
            }
        } while (ret != -EAGAIN);

        ret = mAudioDecoder->send_packet(pPacket, 0);

        if (ret > 0) {
            bool haveError = false;

            if (ret & STATUS_HAVE_ERROR) {
                if (mAudioDecoder->get_error_frame_no() > MAX_DECODE_ERROR_FRAME) {
                    haveError = true;
                }
            }

            if (ret & STATUS_CREATE_FAIL) {
                haveError = true;
            }

            if (haveError) {
                ChangePlayerStatus(PLAYER_ERROR);
                mPNotifier->NotifyError(MEDIA_PLAYER_ERROR_DECODE_AUDIO, "audio decode error");
            }
        }

        return ret;
    }

    void SuperMediaPlayer::ProcessOpenStreamInit(int streamIndex)
    {
        AF_LOGD("ProcessOpenStreamInit ProcessOpenStreamInit start");
        int streamCount = (int) mStreamInfoQueue.size();
        int videoStreams = 0;

        for (int i = 0; i < streamCount; i++) {
            StreamInfo *info = mStreamInfoQueue[i];

            if (info->type == ST_TYPE_VIDEO) {
                videoStreams++;
            }
        }

        if (videoStreams > 1) {
            mAdaptiveVideo = true;
        } else {
            mAdaptiveVideo = false;
        }

        if (mMixMode) {
            if (mCurrentVideoIndex < 0 && mCurrentAudioIndex < 0) {
                unique_ptr<streamMeta> pMeta;
                Stream_meta *meta{};
                int nbSubStream = mDemuxerService->GetNbSubStream(mMainStreamId);

                for (int j = 0; j < nbSubStream; j++) {
                    mDemuxerService->GetStreamMeta(pMeta, GEN_STREAM_ID(mMainStreamId, j), true);
                    meta = (Stream_meta *) (pMeta.get());
                    AF_LOGD("get a stream %d\n", meta->type);

                    if (!mSet.bDisableVideo && meta->type == STREAM_TYPE_VIDEO && mCurrentVideoIndex < 0 &&
                            meta->height > 0 &&
                            meta->attached_pic == 0) {
                        AF_LOGD("get a video stream\n");
                        mCurrentVideoIndex = GEN_STREAM_ID(mMainStreamId, j);
                        mVideoInterlaced = meta->interlaced;
                    } else if (!mSet.bDisableAudio && meta->type == STREAM_TYPE_AUDIO && mCurrentAudioIndex < 0 &&
                               meta->channels > 0) {
                        AF_LOGD("get a audio stream\n");
                        mCurrentAudioIndex = GEN_STREAM_ID(mMainStreamId, j);
                    } else if (meta->type == STREAM_TYPE_SUB && mCurrentSubtitleIndex < 0) {
                        AF_LOGD("get a subtitle stream\n");
                        mCurrentSubtitleIndex = GEN_STREAM_ID(mMainStreamId, j);
                    }
                }
            }
        }

        mPNotifier->CancelNotifyStreamInfo();
        //post before PostMsg(PreparedReq). sdk will get these infos on Prepared callback.
        delete[] mStreamInfos;
        mStreamInfos = new StreamInfo *[mStreamInfoQueue.size()];
        int infoIndex = 0;

        for (StreamInfo *info : mStreamInfoQueue) {
            mStreamInfos[infoIndex++] = info;
        }

        mPNotifier->NotifyStreamInfo(mStreamInfos, static_cast<int>(mStreamInfoQueue.size()));
    }

    void SuperMediaPlayer::setUpAVPath()
    {
        if (!mInited) {
            return;
        }

        if (mCurrentAudioIndex >= 0 && mAudioDecoder == nullptr) {
            AF_LOGD("SetUpAudioPath start");
            int ret = SetUpAudioPath();

            if (ret < 0) {
                AF_LOGE("%s SetUpAudioPath failed,url is %s %s", __FUNCTION__, mSet.url.c_str(), framework_err2_string(ret));
                mDemuxerService->CloseStream(mCurrentAudioIndex);
                mCurrentAudioIndex = -1;
            } else {
            }
        }

        if (mCurrentVideoIndex >= 0 && mVideoDecoder == nullptr) {
            int ret = SetUpVideoPath();

            if (ret < 0) {
                AF_LOGE("%s SetUpVideoPath failed,url is %s %s", __FUNCTION__, mSet.url.c_str(), framework_err2_string(ret));
                mDemuxerService->CloseStream(mCurrentVideoIndex);
                mCurrentVideoIndex = -1;
            }
        }

        if (mCurrentVideoIndex < 0 && mCurrentAudioIndex < 0) {
            ChangePlayerStatus(PLAYER_ERROR);
            mPNotifier->NotifyError(MEDIA_PLAYER_ERROR_DEMUXER_NO_VALID_STREAM, "No valid stream");
        }
    }

    bool SuperMediaPlayer::NeedDrop(int64_t pts, int64_t refer)
    {
        return (pts < refer) && (pts < mDuration - 200 * 1000);
    }

    int SuperMediaPlayer::ReadPacket()
    {
        IAFPacket *pFrame = nullptr;
        std::unique_ptr<IAFPacket> pMedia_Frame{};

        if (mDemuxerService == nullptr) {
            assert(0);
        }

        int index = -1;

        if (HAVE_SUBTITLE && !mSubtitleEOS) {
            if (mBufferController.GetPacketDuration(BUFFER_TYPE_SUBTITLE) <= 0) {
                if (mSubtitleChangedFirstPts != INT64_MIN || 1) {
                    index = mCurrentSubtitleIndex;
                }
            }
        }

        int ret = mDemuxerService->readPacket(pMedia_Frame, index);

        if (pMedia_Frame == nullptr) {
            //  AF_LOGD("Can't read packet %d\n", ret);
            if (ret == 0) {
                mSubtitleEOS = true;

                if (index != -1) {
                    ret = -EAGAIN;
                }
            }

            return ret;
        }

        pFrame = pMedia_Frame.get();
        mUtil.notifyRead(MediaPlayerUtil::readEvent_Got);

        // TODO: get the min first stream pts
        if (pFrame->getInfo().timePosition >= 0 && mMediaStartPts == INT64_MIN
                && pFrame->getInfo().streamIndex != mCurrentSubtitleIndex
                && pFrame->getInfo().streamIndex != mWillChangedSubtitleStreamIndex) {
            mMediaStartPts = pFrame->getInfo().pts - pFrame->getInfo().timePosition;
        }

        if (mSeekFlag && mSeekNeedCatch) {
            if (pFrame->getInfo().timePosition < (mSeekPos - mSet.maxASeekDelta)) {
                // first frame is far away from seek position, don't suppport accurate seek
                mSeekNeedCatch = false;
            }
        }

        int id = GEN_STREAM_INDEX(pFrame->getInfo().streamIndex);

        if (mDuration < 0) {
            unique_ptr<streamMeta> pMeta;
            mDemuxerService->GetStreamMeta(pMeta, pFrame->getInfo().streamIndex, false);
            mDuration = ((Stream_meta *) (pMeta.get()))->duration;
        }

        if (id < mStreamInfoQueue.size()
                && mStreamInfoQueue[id]->type == ST_TYPE_VIDEO
                && mMainStreamId != -1 && id != mMainStreamId) {
            unique_ptr<streamMeta> pMeta;
            Stream_meta *meta{};
            int count = mDemuxerService->GetNbSubStream(id);

            for (int i = 0; i < count; i++) {
                mDemuxerService->GetStreamMeta(pMeta, GEN_STREAM_ID(id, i), true);
                meta = (Stream_meta *) (pMeta.get());
                int streamId = GEN_STREAM_ID(id, i);

                if (meta->type == STREAM_TYPE_VIDEO && meta->height > 0 && streamId != mCurrentVideoIndex) {
                    mWillChangedVideoStreamIndex = streamId;
                } else if (meta->type == STREAM_TYPE_AUDIO && meta->channels > 0 && streamId != mCurrentAudioIndex) {
                    mWillChangedAudioStreamIndex = streamId;
                } else if (meta->type == STREAM_TYPE_SUB && streamId != mCurrentSubtitleIndex) {
                    mWillChangedSubtitleStreamIndex = streamId;
                }
            }

            mMainStreamId = id;
        }

        if (!mInited) {
            ProcessOpenStreamInit(pFrame->getInfo().streamIndex);
            mInited = true;
        }

//        AF_LOGD("read packet pts is %lld,streamIndex is %d duration is %d\n", pFrame->getInfo().pts, pFrame->getInfo().streamIndex,
//                pFrame->getInfo().duration);

        if (pFrame->getInfo().streamIndex == mCurrentVideoIndex ||
                pFrame->getInfo().streamIndex == mWillChangedVideoStreamIndex) {
            if (mMediaFrameCb) {
                // TODO: change to std::unique_ptr<IAFPacket>
                mMediaFrameCb(mMediaFrameCbArg, pMedia_Frame, ST_TYPE_VIDEO);
            }

            mBufferController.AddPacket(move(pMedia_Frame), BUFFER_TYPE_VIDEO);
            mDemuxerService->SetOption("FRAME_RECEIVE", pFrame->getInfo().pts);

            if (mVideoInterlaced == InterlacedType_UNKNOWN) {
                if (mVideoParser == nullptr) {
//FIXME mCurrentVideoMeta can not be update here.
                    mDemuxerService->GetStreamMeta(mCurrentVideoMeta, pFrame->getInfo().streamIndex, false);
                    auto *meta = (Stream_meta *) (mCurrentVideoMeta.get());
                    mVideoParser = new bitStreamParser();
                    mVideoParser->init(meta);
                }

                mVideoParser->parser(pFrame->getData(), static_cast<int>(pFrame->getSize()));
                mVideoInterlaced = mVideoParser->getInterlaced();

                if (mVideoInterlaced != InterlacedType_UNKNOWN) {
                    delete mVideoParser;
                    mVideoParser = nullptr;
                } else {
                    mVideoParserTimes++;

                    if (mVideoParserTimes > 10) {
                        mVideoInterlaced = InterlacedType_NO;
                        delete mVideoParser;
                        mVideoParser = nullptr;
                        mVideoParserTimes = 0;
                    }
                }
            }

            if (mFirstVideoPts == INT64_MIN) {
                mFirstVideoPts = pFrame->getInfo().pts - pFrame->getInfo().timePosition;

                if (!HAVE_AUDIO) {
                    mFirstSeekStartTime = pFrame->getInfo().timePosition;
                }
            }

            if (mSoughtVideoPos == INT64_MIN) {
                mSoughtVideoPos = pFrame->getInfo().timePosition;

                /*
                 * seek would clean the packet which have ExtraData in decoder queue,
                 * so add the ExtraData after seek on key frame
                 */
                if (mAdaptiveVideo && pFrame->getInfo().flags) {
                    unique_ptr<streamMeta> pMeta;
                    mDemuxerService->GetStreamMeta(pMeta, pFrame->getInfo().streamIndex, false);
                    pFrame->setExtraData(((Stream_meta *) (*pMeta))->extradata, ((Stream_meta *) (*pMeta))->extradata_size);
                }
            }

            //this time video bitrate changed
            if (mVideoChangedFirstPts == INT64_MAX && pFrame->getInfo().streamIndex == mWillChangedVideoStreamIndex) {
                AF_LOGD("video stream first pts is %lld", pFrame->getInfo().pts);
                mVideoChangedFirstPts = pFrame->getInfo().pts;
                unique_ptr<streamMeta> pMeta;
                mDemuxerService->GetStreamMeta(pMeta, mWillChangedVideoStreamIndex, false);
                pFrame->setExtraData(((Stream_meta *) (*pMeta))->extradata, ((Stream_meta *) (*pMeta))->extradata_size);
                mCurrentVideoIndex = mWillChangedVideoStreamIndex;
                mWillChangedVideoStreamIndex = -1;
            }
        } else if (pFrame->getInfo().streamIndex == mCurrentAudioIndex || pFrame->getInfo().streamIndex == mWillChangedAudioStreamIndex) {
            // printTimePosition(pFrame->getInfo().timePosition);
            if (mFirstAudioPts == INT64_MIN) {
                mFirstAudioPts = pFrame->getInfo().pts - pFrame->getInfo().timePosition;
                mFirstSeekStartTime = pFrame->getInfo().timePosition;
            }

            if (mSeekFlag && mSeekNeedCatch && NeedDrop(pFrame->getInfo().timePosition, mSeekPos)) {
                return ret;
            }

            if (pFrame->getInfo().streamIndex == mWillChangedAudioStreamIndex) {
                mCurrentAudioIndex = mWillChangedAudioStreamIndex;
                mWillChangedAudioStreamIndex = -1;
            }

            if (mAudioChangedFirstPts == INT64_MAX) {
                int64_t playedTime = mMasterClock.GetTime();

                if (pFrame->getInfo().pts < playedTime) {
                    return ret;
                } else {
                    //recodr 64MAX for audio stream changed for first frame
                    mAudioChangedFirstPts = pFrame->getInfo().pts;
                }
            }

            if (mMediaFrameCb) {
                // TODO: change to std::unique_ptr<IAFPacket>
                mMediaFrameCb(mMediaFrameCbArg, pMedia_Frame, ST_TYPE_AUDIO);
            }

            mBufferController.AddPacket(move(pMedia_Frame), BUFFER_TYPE_AUDIO);
        } else if (pFrame->getInfo().streamIndex == mCurrentSubtitleIndex ||
                   pFrame->getInfo().streamIndex == mWillChangedSubtitleStreamIndex) {
            if (mMediaFrameCb) {
                // TODO: change to std::unique_ptr<IAFPacket>
                mMediaFrameCb(mMediaFrameCbArg, pMedia_Frame, ST_TYPE_SUB);
            }

            mBufferController.AddPacket(move(pMedia_Frame), BUFFER_TYPE_SUBTITLE);
            AF_LOGD("read subtitle pts is %lld", pFrame->getInfo().pts);

            if (pFrame->getInfo().streamIndex == mWillChangedSubtitleStreamIndex) {
                mCurrentSubtitleIndex = mWillChangedSubtitleStreamIndex;
                mWillChangedSubtitleStreamIndex = -1;
            }

            if (mSubtitleChangedFirstPts == INT64_MAX) {
                mSubtitleChangedFirstPts = pFrame->getInfo().pts;
            }
        } else {
            AF_LOGD("unknown stream %x, read packet pts is %lld\n", pFrame->getInfo().streamIndex,
                    pFrame->getInfo().pts);
        }

        if (mWillSwitchVideo) {
            int videoCount = 0;
            int64_t startTime = mBufferController.FindSeamlessPointTimePosition(BUFFER_TYPE_VIDEO, videoCount);

            if (startTime == 0 || videoCount < 40) {
                return ret;
            }

            if (mMixMode) {
                int64_t startTimeA = mBufferController.FindSeamlessPointTimePosition(BUFFER_TYPE_AUDIO, videoCount);

                if (startTimeA == 0 || videoCount < 40) {
                    return ret;
                }

                startTime = std::max(startTime, startTimeA);
            }

            SwitchVideo(startTime);
            mWillSwitchVideo = false;
        }

        return ret;
    }

    void SuperMediaPlayer::printTimePosition(int64_t time) const
    {
        if (time > 0) {
            time /= 1000;
            int timeM = static_cast<int>((time / 1000) / 60);
            int timeS = static_cast<int>((time / 1000) % 60);
            int timeMS = static_cast<int>(time - timeM * 60000 - timeS * 1000);
            AF_LOGD("timePosition is %d:%d:%d\n", timeM, timeS, timeMS);
        }
    }

    void SuperMediaPlayer::FlushAudioPath()
    {
        if (mAudioRender) {
            mAudioRender->flush();
        }

        if (mAudioDecoder != nullptr) {
            mAudioDecoder->flush();
        }

        audioDecoderEOS = false;

        //flush frame queue
        while (!mAudioFrameQue.empty()) {
            mAudioFrameQue.pop_front();
        }

        mPlayedAudioPts = INT64_MIN;
        mAudioPtsRevert = false;
        mAudioTime.startTime = 0;
        mAudioTime.deltaTime = 0;
        mAudioTime.deltaTimeTmp = 0;
        mAudioPacket = nullptr;
    }

    void SuperMediaPlayer::FlushVideoPath()
    {
        if (mVideoRender) {
            unique_ptr<IAFFrame> frame{nullptr};
            mVideoRender->renderFrame(frame);
        }

        if (mVideoDecoder) {
            mVideoDecoder->flush();
        }

        videoDecoderEOS = false;
        //flush frame queue

        while (!mVideoFrameQue.empty()) {
            ProcessVideoRenderedMsg(mVideoFrameQue.front()->getInfo().pts, af_getsteady_ms(), nullptr);
            mVideoFrameQue.pop();
        }

        mPlayedVideoPts = INT64_MIN;
        mCurVideoPts = INT64_MIN;
        videoDecoderFull = false;
        mVideoPtsRevert = false;
        mVideoPacket = nullptr;
        dropLateVideoFrames = false;
    }

    void SuperMediaPlayer::FlushSubtitleInfo()
    {
        while (!mSubtitleShowedQueue.empty()) {
            if (mSubtitleShowedQueue.front()) {
                mPNotifier->NotifySubtitleEvent(subTitle_event_hide, mSubtitleShowedQueue.front().release(), 0, nullptr);
            }

            mSubtitleShowedQueue.pop_front();
        }

        mSubtitleShowedQueue.clear();
        mSubtitleShowIndex = 0;
        mSubtitleEOS = false;
    }

    void SuperMediaPlayer::PostBufferPositionMsg()
    {
        if (mPlayStatus == PLAYER_PAUSED || mPlayStatus == PLAYER_PLAYING) {
            int64_t duration;

            if (isSeeking()) {
                duration = 0;
            } else {
                duration = getPlayerBufferDuration(false);
            }

            if (duration >= 0) {
                mBufferPosition = getCurrentPosition() + duration;

                if (mEof) {
                    mBufferPosition = mDuration;
                }

                mPNotifier->NotifyBufferPosition((mBufferPosition <= mDuration ? mBufferPosition : mDuration) / 1000);
            }
        }
    }

    int64_t SuperMediaPlayer::getPlayerBufferDuration(bool gotMax)
    {
        int64_t durations[3] = {-1, -1, -1};
        int i = 0;
        int64_t duration = -1;

        if (HAVE_VIDEO) {
            int64_t &duration_c = durations[i++];
            duration_c = mBufferController.GetPacketDuration(BUFFER_TYPE_VIDEO);

//            AF_LOGD("videoDuration is %lld\n",videoDuration);
            if (duration_c < 0 && !HAVE_AUDIO) {
                duration_c =
                    mBufferController.GetPacketLastPTS(BUFFER_TYPE_VIDEO) -
                    mBufferController.GetPacketPts(BUFFER_TYPE_VIDEO);

                if (duration_c <= 0) {
                    duration_c = (int64_t) mBufferController.GetPacketSize(BUFFER_TYPE_VIDEO) * 40 * 1000;
                }
            }
        }

        if (HAVE_AUDIO) {
            int64_t &duration_c = durations[i++];
            duration_c = mBufferController.GetPacketDuration(BUFFER_TYPE_AUDIO);
//            AF_LOGD("audioDuration is %lld\n",audioDuration);
        }

        /*
         *  Do not let player loading when switching subtitle, we'll read subtitle first
         *  in ReadPacket()
         */
        if (HAVE_SUBTITLE && !mSubtitleEOS && mSubtitleChangedFirstPts == INT64_MIN) {
            int64_t &duration_c = durations[i++];
            duration_c = mBufferController.GetPacketDuration(BUFFER_TYPE_SUBTITLE);
        }

        int num = i;

        for (i = 0; i < num; i++) {
            if (duration < 0) {
                duration = durations[i];
            } else {
                duration = gotMax ? std::max(duration, durations[i]) : std::min(duration, durations[i]);
            }
        }

        return duration;
    }

    bool SuperMediaPlayer::SeekInCache(int64_t pos)
    {
        int64_t audioLastPos = mBufferController.GetPacketLastTimePos(BUFFER_TYPE_AUDIO);
        int64_t videoLastPos = mBufferController.GetPacketLastTimePos(BUFFER_TYPE_VIDEO);
        int64_t minLastPos = -1;

        if (HAVE_VIDEO && HAVE_AUDIO) {
            minLastPos = audioLastPos < videoLastPos ? audioLastPos : videoLastPos;
        } else if (HAVE_VIDEO) {
            minLastPos = videoLastPos;
        } else if (HAVE_AUDIO) {
            minLastPos = audioLastPos;
        } else {
            // no video and audio ?
        }

        //seek before
        if (pos < getCurrentPosition()) {
            return false;
        }

        //seek bigger than last frame
        if (pos > minLastPos) {
            return false;
        }

        int64_t keyPosBefore = INT64_MIN;

        //can not find last key video pts,return
        if (HAVE_VIDEO) {
            keyPosBefore = mBufferController.GetKeyTimePositionBefore(BUFFER_TYPE_VIDEO, pos);
        } else if (HAVE_AUDIO) {
            keyPosBefore = mBufferController.GetKeyTimePositionBefore(BUFFER_TYPE_AUDIO, pos);
        }

        if (keyPosBefore == INT64_MIN) {
            return false;
        }

        mBufferController.ClearPacketBeforeTimePos(BUFFER_TYPE_ALL, keyPosBefore);
        mSoughtVideoPos = keyPosBefore;
        return true;
    }

    void SuperMediaPlayer::SwitchVideo(int64_t startTime)
    {
        AF_LOGD("video change find start time is %lld", startTime);
        int ret = mDemuxerService->OpenStream(mWillChangedVideoStreamIndex);

        if (ret < 0) {
            AF_LOGD("video", "switch video open stream failed,stream index %d\n",
                    mCurrentVideoIndex);
            return;
        }

        if (mMixMode) {
            mDemuxerService->CloseStream(GEN_STREAM_INDEX(mCurrentVideoIndex));
        } else {
            mDemuxerService->CloseStream(mCurrentVideoIndex);
        }

        mDemuxerService->Seek(startTime / 1000 * 1000, 0, mWillChangedVideoStreamIndex);

        if (mMixMode) {
            mBufferController.ClearPacketAfterTimePosition(BUFFER_TYPE_AV, startTime);
        } else {
            mBufferController.ClearPacketAfterTimePosition(BUFFER_TYPE_VIDEO, startTime);
        }

        mWillSwitchVideo = false;
        mVideoChangedFirstPts = INT64_MAX;
        mEof = false;
    }

    int64_t SuperMediaPlayer::getAudioPlayTimeStampCB(void *arg)
    {
        SuperMediaPlayer *pHandle = static_cast<SuperMediaPlayer *>(arg);
        return pHandle->getAudioPlayTimeStamp();
    }

    int64_t SuperMediaPlayer::getAudioPlayTimeStamp()
    {
        if (mAudioRender == nullptr) {
            return INT64_MIN;
        }

        if (mSeekFlag) {
            return INT64_MIN;
        }

        int64_t aoutPos;
        aoutPos = mAudioRender->getPosition();
        return mAudioTime.startTime + mAudioTime.deltaTime + aoutPos;
    }

    void SuperMediaPlayer::GetVideoResolution(int &width, int &height)
    {
        width = mVideoWidth;
        height = mVideoHeight;
    }

    void SuperMediaPlayer::GetVideoRotation(int &rotation)
    {
        rotation = mVideoRotation;
    }

    int SuperMediaPlayer::setUpAudioDecoder(const Stream_meta *meta)
    {
        if (mAudioDecoder != nullptr) {
            return 0;
        }

        int ret = 0;

        if (meta->samplerate <= 0) { // meta.frame_size maybe 0 when playing artp
            ret = 0;
            return 0;
        }

        if (meta->duration > mDuration) {
            mDuration = meta->duration;
        }

        //setVolume to current setting after create new.
        SetVolume(mSet.mVolume);

        if (mSet.bMute) {
            ProcessMuteMsg();
        }

        mAudioDecoder = decoderFactory::create(meta->codec, DECFLAG_SW, 0);

        if (mAudioDecoder == nullptr) {
            ret = gen_framework_errno(error_class_codec, codec_error_audio_not_support);
            mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_AUDIO_CODEC_NOT_SUPPORT, framework_err2_string(ret));
            return ret;
        }

        ret = mAudioDecoder->open(meta, nullptr, 0);

        if (ret < 0) {
            AF_LOGE("mAudioDecoder init error %d\n", ret);
            mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_AUDIO_DECODER_DEVICE_ERROR, framework_err2_string(ret));
            mAudioDecoder = nullptr;
            return ret;
        }

        return ret;
    }

    int SuperMediaPlayer::SetUpAudioPath()
    {
        if (mBufferController.IsPacketEmtpy(BUFFER_TYPE_AUDIO)) {
            return 0;
        }

        unique_ptr<streamMeta> pMeta{};
        mDemuxerService->GetStreamMeta(pMeta, mCurrentAudioIndex, false);
        Stream_meta *meta = (Stream_meta *) (pMeta.get());
        int ret = setUpAudioDecoder(meta);

        if (ret < 0) {
            return ret;
        }

        IAFFrame::audioInfo info{};
        info.channels = meta->channels;
        info.sample_rate = meta->samplerate;
        info.format = meta->sample_fmt;
        info.nb_samples = meta->frame_size;
        info.channel_layout = meta->channel_layout;
        setUpAudioRender(info);
        return ret;
    }

    int SuperMediaPlayer::setUpAudioRender(const IAFFrame::audioInfo &info)
    {
        if (mAudioRender == nullptr) {
            mAudioRender = AudioRenderFactory::create();
        }

        assert(mAudioRender);
        mAudioRender->setListener(&mAudioRenderCB);
        int audioInitRet = mAudioRender->init(&info);

        if (audioInitRet < 0) {
            AF_LOGE("AudioOutHandle Init Error is %d", audioInitRet);
            // don't release audio handle because we only new it in constructor
            // PS: we should try to recover it later, or notify error
//                    mAudioOutHandle = 0;
            mCurrentAudioIndex = -1;
            return -1;
        } else {
            mAudioRender->setSpeed(mSet.rate);
            mAudioRender->mute(mSet.bMute);
            mAudioRender->setVolume(mSet.mVolume);
            return 0;
        }
    }

    int SuperMediaPlayer::SetUpVideoPath()
    {
        if (mVideoDecoder) {
            return 0;
        }

        if (mBufferController.IsPacketEmtpy(BUFFER_TYPE_VIDEO)) {
            return 0;
        }

        mDemuxerService->GetStreamMeta(mCurrentVideoMeta, mCurrentVideoIndex, false);
        auto *meta = (Stream_meta *) (mCurrentVideoMeta.get());

        if (mVideoWidth != meta->width || mVideoHeight != meta->height) {
            mVideoWidth = meta->width;
            mVideoHeight = meta->height;
            mVideoRotation = meta->rotate;
            mPNotifier->NotifyVideoSizeChanged(mVideoWidth, mVideoHeight);
        }

        if (mSet.mView == nullptr) {
            return 0;
        }

        if (mVideoInterlaced == InterlacedType_UNKNOWN) {
            AF_LOGW("Wait for parser video interlaced Type");
            return 0;
        }

        int ret = 0;
        bool bHW = false;
        AF_LOGD("SetUpVideoPath start");

        if (mSet.bEnableHwVideoDecode) {
            switch (meta->codec) {
                case AF_CODEC_ID_H264: {
                    string value = getProperty("ro.video.dec.h264");
                    bHW = !(value == "OFF");
                    break;
                }

                default:
                    bHW = true;
                    break;
            }

            if (mAppStatus == APP_BACKGROUND) {
                AF_LOGW("create video render in background");
            }
        }

        if (meta->interlaced == InterlacedType_UNKNOWN) {
            meta->interlaced = mVideoInterlaced;
        }

        if (!mSet.bEnableTunnelRender) {
            CreateVideoRender();
        }

        if (mVideoRender) {
            IVideoRender::Rotate finalRotate = IVideoRender::Rotate::Rotate_None;

            if (meta->rotate == 0) {
                finalRotate = IVideoRender::Rotate::Rotate_None;
            } else if (meta->rotate == 90) {
                finalRotate = IVideoRender::Rotate::Rotate_90;
            } else if (meta->rotate == 180) {
                finalRotate = IVideoRender::Rotate::Rotate_180;
            } else if (meta->rotate == 270) {
                finalRotate = IVideoRender::Rotate::Rotate_270;
            }

            mVideoRender->setVideoRotate(finalRotate);
        }

        //re set view in case for not set view before
        if (mSet.mView) {
            if (mVideoRender) {
                mVideoRender->setDisPlay(mSet.mView);
            }
        }

        ret = CreateVideoDecoder(bHW, *meta);

        if (ret < 0) {
            if (bHW) {
                ret = CreateVideoDecoder(false, *meta);
            }
        }

        if (ret < 0) {
            AF_LOGE("%s CreateVideoDecoder failed, error msg is  %s", __FUNCTION__, framework_err2_string(ret));

            if (ret == gen_framework_errno(error_class_codec, codec_error_video_not_support)) {
                mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_VIDEO_CODEC_NOT_SUPPORT, framework_err2_string(ret));
            } else if (ret == gen_framework_errno(error_class_codec, codec_error_video_device_error)) {
                mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_VIDEO_DECODER_DEVICE_ERROR, framework_err2_string(ret));
            }

            return ret;
        }

        if (mVideoDecoder->getFlags() & DECFLAG_HW) {
        } else {
            if (mSet.bEnableHwVideoDecode) {
                mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_SW_VIDEO_DECODER, "Switch to software video decoder");
            }
        }

//
//#ifdef WIN32
//
//        if (mVideoDecodeType == AlivcVideoCodecHardware) {
//            if (meta.width >= 7680 && meta.height >= 4320) {
//                mVideoDecoder->enable_skip_frame(true, 60.0);
//            }
//
//            m_dxva2Render.InitRender(mSet.mView, mVideoDecoder->get_video_device(), meta.width, meta.height);
//        }
//
//#endif

        if (meta->duration > mDuration) {
            mDuration = meta->duration;
        }

        return ret;
    }

    bool SuperMediaPlayer::CreateVideoRender()
    {
        if (nullptr != mVideoRender) {
            return true;
        }

        // lock mAppStatusMutex before mCreateMutex
        std::lock_guard<std::mutex> uMutex(mCreateMutex);
        mVideoRender = videoRenderFactory::create();
        mVideoRender->setScale(convertScaleMode(mSet.scaleMode));
        mVideoRender->setRotate(convertRotateMode(mSet.rotateMode));
        mVideoRender->setBackgroundColor(mSet.mVideoBackgroundColor);
        mVideoRender->setFlip(convertMirrorMode(mSet.mirrorMode));
        mVideoRender->setDisPlay(mSet.mView);
        mVideoRender->setRenderResultCallback([this](int64_t pts, bool rendered) -> void {
            VideoRenderCallback(this, pts, nullptr);
        });
        int renderRet = mVideoRender->init();

        if (renderRet != 0) {
            // for windows init failed, which may need change render type in future.
            mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_VIDEO_RENDER_INIT_ERROR, "init video render failed");
        }

        mVideoRender->setSpeed(mSet.rate);
        return true;
    }

    int SuperMediaPlayer::CreateVideoDecoder(bool bHW, Stream_meta &meta)
    {
        int ret;
        uint64_t decFlag = 0;

        if (bHW) {
            decFlag |= DECFLAG_HW;
        } else {
            decFlag |= DECFLAG_SW;
        }

        if (mAdaptiveVideo) {
            decFlag |= DECFLAG_ADAPTIVE;
        }

        {
            std::lock_guard<std::mutex> uMutex(mCreateMutex);
            mVideoDecoder = decoderFactory::create(meta.codec, decFlag, max(meta.height, meta.width));
        }

        if (mVideoDecoder == nullptr) {
            return gen_framework_errno(error_class_codec, codec_error_video_not_support);
        }

        void *view = nullptr;

        if (bHW) {
            if (mSet.bEnableTunnelRender) {
                view = mSet.mView;
                decFlag |= DECFLAG_DIRECT;
            } else {
                if (mVideoRender) {
                    view = mVideoRender->getSurface();
                }
            }
        }

        if (mSet.bLowLatency) {
            // artp disable b frame to reduce delay at present
            decFlag |= DECFLAG_OUTPUT_FRAME_ASAP;
        }

        ret = mVideoDecoder->open(&meta, view, decFlag);

        if (ret < 0) {
            AF_LOGE("config decoder error ret= %d \n", ret);
            //TODO:PostErrorMsg
            std::lock_guard<std::mutex> uMutex(mCreateMutex);
            mVideoDecoder = nullptr;
            return gen_framework_errno(error_class_codec, codec_error_video_device_error);
        }

        return 0;
    }


    void SuperMediaPlayer::Reset()
    {
        mCurrentVideoIndex = -1;
        mCurrentAudioIndex = -1;
        mCurrentSubtitleIndex = -1;
        mVideoWidth = 0;
        mVideoHeight = 0;
        mVideoRotation = 0;
        mDuration = INT64_MIN;
        mBufferPosition = 0;
        mSeekPos = INT64_MIN;
        mPlayedVideoPts = INT64_MIN;
        mPlayedAudioPts = INT64_MIN;
        mSeekFlag = false;
        mFirstAudioPts = INT64_MIN;
        mFirstVideoPts = INT64_MIN;
        mMediaStartPts = INT64_MIN;
        mEof = false;
        mFirstBufferFlag = true;
        mBufferingFlag = false;
        mCurVideoPts = INT64_MIN;
        mLastAudioFrameDuration = INT64_MIN;
        mTimeoutStartTime = INT64_MIN;
        mSubtitleShowIndex = 0;
        mWillChangedVideoStreamIndex = -1;
        mWillChangedAudioStreamIndex = -1;
        mWillChangedSubtitleStreamIndex = -1;
        mBufferIsFull = false;
        mWillSwitchVideo = false;
        mMixMode = false;
        mFirstRendered = false;
        mInited = false;
        mSeekNeedCatch = false;
        mMainStreamId = -1;
        mRemovedFirstAudioPts = INT64_MIN;
        mFirstSeekStartTime = 0;
        mAudioChangedFirstPts = INT64_MIN;
        mVideoChangedFirstPts = INT64_MIN;
        mSubtitleChangedFirstPts = INT64_MIN;
        mSoughtVideoPos = INT64_MIN;
        mPlayingPosition = 0;
        mFirstReadPacketSucMS = 0;
        mCanceled = false;
        mPNotifier->Enable(true);
        mMasterClock.reset();
        FlushSubtitleInfo();
        mSubtitleShowedQueue.clear();
        mBSReadCb = nullptr;
        mBSCbArg = nullptr;
        mBSSeekCb = nullptr;
        mBSCbArg = nullptr;
        mUtil.reset();
        mVideoInterlaced = InterlacedType_UNKNOWN;
        mVideoParserTimes = 0;
        mVideoPtsRevert = mAudioPtsRevert = false;
        mLowMem = false;
        mCurrentVideoMeta = nullptr;
        mAdaptiveVideo = false;
        dropLateVideoFrames = false;
        mBRendingStart = false;
        mSubtitleEOS = false;

        if (mVideoRender) {
            mVideoRender->setSpeed(1);
        }
    }

    int SuperMediaPlayer::GetCurrentStreamIndex(StreamType type)
    {
        int streamIndex = -1;

        if (mMixMode && type != ST_TYPE_SUB) {
            if (HAVE_VIDEO) {
                streamIndex = GEN_STREAM_INDEX(mCurrentVideoIndex);
            } else if (HAVE_AUDIO) {
                streamIndex = GEN_STREAM_INDEX(mCurrentAudioIndex);
            }
        } else {
            switch (type) {
                case ST_TYPE_AUDIO:
                    streamIndex = mCurrentAudioIndex;
                    break;

                case ST_TYPE_VIDEO:
                    streamIndex = mCurrentVideoIndex;
                    break;

                case ST_TYPE_SUB:
                    streamIndex = mCurrentSubtitleIndex;
                    break;

                default:
                    break;
            }
        }

        return streamIndex;
    }

    StreamInfo *SuperMediaPlayer::GetCurrentStreamInfo(StreamType type)
    {
        int streamIndex = GetCurrentStreamIndex(type);

        if (streamIndex != -1) {
            for (StreamInfo *info : mStreamInfoQueue) {
                if (info->streamIndex == streamIndex) {
                    return info;
                }
            }
        }

        return nullptr;
    }

    void SuperMediaPlayer::ProcessPrepareMsg()
    {
        AF_LOGD("ProcessPrepareMsg start");
        int ret;

        if (mSet.url.empty() && mBSReadCb == nullptr) {
            AF_LOGD("ProcessPrepareMsg url is empty");
            ChangePlayerStatus(PLAYER_ERROR);
            mPNotifier->NotifyError(MEDIA_PLAYER_ERROR_DATASOURCE_EMPTYURL, "Prepare url is empty");
            return;
        }

        if (mPlayStatus != PLAYER_INITIALZED && mPlayStatus != PLAYER_STOPPED) {
            AF_LOGD("ProcessPrepareMsg status is %d", mPlayStatus.load());
            return;
        }

        mPlayStatus = PLAYER_PREPARINIT;
        bool noFile = false;

        if (!(mBSReadCb != nullptr && mBSSeekCb != nullptr && mBSCbArg != nullptr)) {
            if (!mSet.url.empty()) {
                ret = openUrl();

                if (ret < 0) {
                    AF_LOGD("%s mDataSource open failed,url is %s %s", __FUNCTION__, mSet.url.c_str(),
                            framework_err2_string(ret));

                    if (ret == FRAMEWORK_ERR_EXIT) {
                        // stop by user.
                        //ChangePlayerStatus(PLAYER_STOPPED);
                        return;
                    } else if (ret == FRAMEWORK_ERR_PROTOCOL_NOT_SUPPORT) {
                        noFile = true;
                    } else {
                        NotifyError(ret);
                        return;
                    }
                }
            }
        }

        if (mCanceled) {
            return;
        }

        {
            std::lock_guard<std::mutex> locker(mCreateMutex);
            mDemuxerService = new demuxer_service(mDataSource);
            mDemuxerService->setOptions(&mSet.mOptions);
        }

        std::function<void(std::string, std::string)> demuxerCB = [this](const std::string & key, const std::string & value) -> void {
            this->OnDemuxerCallback(key, value);
        };
        mDemuxerService->setDemuxerCb(demuxerCB);
        mDemuxerService->setNoFile(noFile);

        if (!noFile) {
            mDemuxerService->SetDataCallBack(mBSReadCb, mBSCbArg, mBSSeekCb, mBSCbArg, nullptr);
        }


        //prepare之前seek
        if (mSeekPos > 0) {
            mDemuxerService->Seek(mSeekPos, 0, -1);
            mSeekFlag = true;
        } else {
            ResetSeekStatus();
        }

        AF_LOGD("initOpen start");
        ret = mDemuxerService->createDemuxer((mBSReadCb || noFile) ? demuxer_type_bit_stream : demuxer_type_unknown);

        // TODO: video tool box HW decoder not merge the header
        if (mDemuxerService->getDemuxerHandle()) {
#ifdef __APPLE__
            mDemuxerService->getDemuxerHandle()->setBitStreamFormat(false, false);
#else
            mDemuxerService->getDemuxerHandle()->setBitStreamFormat(true, true);
#endif
            if (noFile) {
                IDataSource::SourceConfig config;
                mDataSource->Get_config(config);
                mDemuxerService->getDemuxerHandle()->setDataSourceConfig(config);
            }
        }

        //step2: Demuxer init and getstream index
        ret = mDemuxerService->initOpen((mBSReadCb || noFile) ? demuxer_type_bit_stream : demuxer_type_unknown);

        if (ret < 0) {
            if (ret != FRAMEWORK_ERR_EXIT && !mCanceled) {
                NotifyError(ret);
            }

            return;
        }

        int nbStream = mDemuxerService->GetNbStreams();
        AF_LOGD("Demuxer service get nubmer streams is %d", nbStream);
        unique_ptr<streamMeta> pMeta;
        int bandWidthNearStreamIndex = -1;
        int minBandWidthDelta = INT_MAX;
        int mDefaultBandWidth = mSet.mDefaultBandWidth;

        for (int i = 0; i < nbStream; ++i) {
            mDemuxerService->GetStreamMeta(pMeta, i, false);
            auto *meta = (Stream_meta *) (pMeta.get());

            if (meta->type == STREAM_TYPE_MIXED) {
                mMixMode = true;
            }

            if (meta->type == STREAM_TYPE_MIXED || meta->type == STREAM_TYPE_VIDEO) {
                int metaBandWidth = (int) meta->bandwidth;

                if (abs(mDefaultBandWidth - metaBandWidth) < minBandWidthDelta) {
                    bandWidthNearStreamIndex = i;
                    minBandWidthDelta = abs(mDefaultBandWidth - metaBandWidth);
                }
            }
        }

        for (int i = 0; i < nbStream; ++i) {
            int openStreamRet = 0;
            mDemuxerService->GetStreamMeta(pMeta, i, false);
            auto *meta = (Stream_meta *) (pMeta.get());
            auto *info = new StreamInfo();
            info->streamIndex = i;
            info->subtitleLang = nullptr;
            info->audioLang = nullptr;
            info->description = nullptr;
            AF_LOGD("get a stream %d\n", meta->type);

            if (!mSet.bDisableVideo && meta->type == STREAM_TYPE_VIDEO) {
                info->type = ST_TYPE_VIDEO;
                info->videoWidth = meta->width;
                info->videoHeight = meta->height;
                info->videoBandwidth = (int) meta->bandwidth;

                if (meta->description) {
                    info->description = strdup((const char *) meta->description);
                }

                mStreamInfoQueue.push_back(info);
                mVideoInterlaced = meta->interlaced;

                if (mCurrentVideoIndex < 0 && !mMixMode && meta->attached_pic == 0) {
                    if (bandWidthNearStreamIndex == i) {
                        AF_LOGD("get a video stream\n");
                        openStreamRet = mDemuxerService->OpenStream(i);
                        mCurrentVideoIndex = i;
                        mDemuxerService->GetStreamMeta(mCurrentVideoMeta, i, false);
                    }
                }
            } else if (!mSet.bDisableAudio && meta->type == STREAM_TYPE_AUDIO) {
                info->type = ST_TYPE_AUDIO;

                if (meta->lang) {
                    info->audioLang = strdup((const char *) meta->lang);
                }

                if (meta->description) {
                    info->description = strdup((const char *) meta->description);
                }

                info->nChannels = meta->channels;
                info->sampleFormat = meta->sample_fmt;
                info->sampleRate = meta->samplerate;
                mStreamInfoQueue.push_back(info);

                if (mCurrentAudioIndex < 0 && !mMixMode) {
                    AF_LOGD("get a audio stream\n");
                    openStreamRet = mDemuxerService->OpenStream(i);
                    mCurrentAudioIndex = i;
                }
            } else if (meta->type == STREAM_TYPE_SUB) {
                info->type = ST_TYPE_SUB;

                if (meta->lang) {
                    info->subtitleLang = strdup((const char *) meta->lang);
                }

                if (meta->description) {
                    info->description = strdup((const char *) meta->description);
                }

                mStreamInfoQueue.push_back(info);

                if (mCurrentSubtitleIndex < 0) {
                    AF_LOGD("get a subtitle stream\n");
                    openStreamRet = mDemuxerService->OpenStream(i);
                    mCurrentSubtitleIndex = i;
                }
            } else if (meta->type == STREAM_TYPE_MIXED) {
                info->type = ST_TYPE_VIDEO;
                info->streamIndex = i;
                info->videoBandwidth = (int) meta->bandwidth;
                info->videoWidth = meta->width;
                info->videoHeight = meta->height;
                AF_LOGD("STREAM_TYPE_MIXED bandwidth is %llu", meta->bandwidth);

                if (mMainStreamId >= 0) {
                    AF_LOGD("already readed stream");
                } else if (bandWidthNearStreamIndex == i) {
                    mMixMode = true;
                    openStreamRet = mDemuxerService->OpenStream(i);
                    mMainStreamId = i;
                }

                mStreamInfoQueue.push_back(info);
            } else {
                delete info;
            }

            if (openStreamRet < 0) {
                ChangePlayerStatus(PLAYER_ERROR);
                mPNotifier->NotifyError(MEDIA_PLAYER_ERROR_DEMUXER_OPENSTREAM, "open stream failed");
                return;
            }
        }

        if (!HAVE_VIDEO) {
            mSeekNeedCatch = false;
        }

        AF_LOGD("initOpen end");
        mDemuxerService->start();
        ChangePlayerStatus(PLAYER_PREPARING);
        mTimeoutStartTime = INT64_MIN;
    }

    int SuperMediaPlayer::openUrl()
    {
        IDataSource::SourceConfig config{};
        config.low_speed_time_ms = mSet.timeout_ms;
        config.low_speed_limit = 1;
        //   config.max_time_ms = mSet.timeout;
        config.connect_time_out_ms = mSet.timeout_ms;
        config.http_proxy = mSet.http_proxy;
        config.refer = mSet.refer;
        config.userAgent = mSet.userAgent;
        config.customHeaders = mSet.customHeaders;
        config.listener = &mSourceListener;
        mSourceListener.enableRetry();

        if (mCanceled) {
            return FRAMEWORK_ERR_EXIT;
        }

        {
            std::lock_guard<std::mutex> locker(mCreateMutex);
            mDataSource = dataSourcePrototype::create(mSet.url, &mSet.mOptions);
        }

        if (mDataSource) {
            mDataSource->Set_config(config);
            int ret = mDataSource->Open(0);
            return ret;
        }

        return -1;
    }

    void SuperMediaPlayer::ProcessSwitchStreamMsg(int index)
    {
        if (mDemuxerService == nullptr) {
            return;
        }

        Stream_type type = STREAM_TYPE_UNKNOWN;
        int i;
        int number = mDemuxerService->GetNbStreams();

        for (i = 0; i < number; i++) {
            unique_ptr<streamMeta> pMeta;
            mDemuxerService->GetStreamMeta(pMeta, i, false);
            auto *meta = (Stream_meta *) (pMeta.get());

            if (index == i) {
                type = meta->type;
                break;
            }
        }

        if (i >= number) {
            AF_LOGW("no such stream\n");
            return;
        }

        if (mDuration == 0) {
            int id = GEN_STREAM_INDEX(index);

            if (mMainStreamId == -1 || mMainStreamId == id) {
                AF_LOGD("current stream index is the same");
                return;
            }

            mVideoChangedFirstPts = INT64_MAX;
            mAudioChangedFirstPts = INT64_MAX;
            mEof = false;
            mDemuxerService->SwitchStreamAligned(mMainStreamId, id);
            return;
        }

        if (type == STREAM_TYPE_MIXED) {
            int id = GEN_STREAM_INDEX(index);

            if (mMainStreamId == -1 || mMainStreamId == id) {
                AF_LOGD("current stream index is the same");
                return;
            }

            mVideoChangedFirstPts = INT64_MAX;
            mAudioChangedFirstPts = INT64_MAX;
            mEof = false;
            switchVideoStream(id, type);
            return;
        }

        if (type == STREAM_TYPE_SUB && mCurrentSubtitleIndex >= 0 &&
                mCurrentSubtitleIndex != index) {
            return switchSubTitle(index);
        } else if (type == STREAM_TYPE_AUDIO && mCurrentAudioIndex >= 0 &&
                   mCurrentAudioIndex != index) {
            return switchAudio(index);
        } else if (type == STREAM_TYPE_VIDEO && mCurrentVideoIndex >= 0 &&
                   mCurrentVideoIndex != index) {
            return switchVideoStream(index, type);
        }
    }

    void SuperMediaPlayer::switchVideoStream(int index, Stream_type type)
    {
        int count = (int) mStreamInfoQueue.size();
        StreamInfo *currentInfo = nullptr;
        StreamInfo *willChangeInfo = nullptr;
        int i;
        int currentId = mCurrentVideoIndex;

        if (type == STREAM_TYPE_MIXED) {
            currentId = GEN_STREAM_INDEX(mCurrentVideoIndex);
        }

        for (i = 0; i < count; i++) {
            StreamInfo *info = mStreamInfoQueue[i];

            if (info->streamIndex == index) {
                willChangeInfo = info;
            }

            if (currentId == info->streamIndex) {
                currentInfo = info;
            }
        }

        if (!willChangeInfo || !currentInfo) {
            return;
        }

        AF_LOGD("video change video bitrate before is %d,after is %d",
                currentInfo->videoBandwidth, willChangeInfo->videoBandwidth);
        //TODO: different strategy
        mWillChangedVideoStreamIndex = index;
        mVideoChangedFirstPts = INT64_MAX;

        if (willChangeInfo->videoBandwidth < currentInfo->videoBandwidth) {
            mDemuxerService->SwitchStreamAligned(currentId, index);
        } else {
            mMixMode = (type == STREAM_TYPE_MIXED);
            int videoCount = 0;
            int64_t startTime = mBufferController.FindSeamlessPointTimePosition(BUFFER_TYPE_VIDEO, videoCount);

            if (startTime == 0 || videoCount < 40) {
                mWillSwitchVideo = true;
                return;
            }

            if (mMixMode) {
                int64_t startTimeA = mBufferController.FindSeamlessPointTimePosition(BUFFER_TYPE_AUDIO, videoCount);

                if (startTimeA == 0 || videoCount < 40) {
                    mWillSwitchVideo = true;
                    return;
                }

                startTime = std::max(startTime, startTimeA);
            }

            SwitchVideo(startTime);
        }
    }

    void SuperMediaPlayer::switchAudio(int index)
    {
        // TODO: use position to seek demuxer ,and drop the late packet
        int ret = mDemuxerService->OpenStream(index);

        if (ret < 0) {
            AF_LOGD("subtitle", "switch audio open stream failed,stream index %d\n",
                    index);
            return;
        }

        mDemuxerService->CloseStream(mCurrentAudioIndex);
        mAudioChangedFirstPts = INT64_MAX;
        mCurrentAudioIndex = index;
        int64_t playTime = mMasterClock.GetTime();
        int64_t pts = playTime - mFirstAudioPts;
        mMasterClock.setReferenceClock(nullptr, nullptr);
        mBufferController.ClearPacket(BUFFER_TYPE_AUDIO);
        mEof = false;
        FlushAudioPath();
        mDemuxerService->Seek(pts, 0, index);
        mPlayedAudioPts = INT64_MIN;
    }

    void SuperMediaPlayer::switchSubTitle(int index)
    {
        int ret = mDemuxerService->OpenStream(index);

        if (ret < 0) {
            AF_LOGD("subtitle", "switch subtitle open stream failed,stream index %d\n", index);
            return;
        }

        mSubtitleChangedFirstPts = INT64_MAX;
        mDemuxerService->CloseStream(mCurrentSubtitleIndex);
        mCurrentSubtitleIndex = index;
        mBufferController.ClearPacket(BUFFER_TYPE_SUBTITLE);
        mEof = false;
        mSubtitleEOS = false;
        FlushSubtitleInfo();
        mDemuxerService->Seek(getCurrentPosition(), 0, index);
    }

    void SuperMediaPlayer::ProcessStartMsg()
    {
        if (mPlayStatus == PLAYER_PAUSED || mPlayStatus == PLAYER_PREPARED ||
                mPlayStatus == PLAYER_COMPLETION) {
            mUtil.reset();

            if (mPlayStatus != PLAYER_PAUSED) {
                if (HAVE_AUDIO) {
                    mMasterClock.setTime(mFirstAudioPts);
                } else {
                    mMasterClock.setTime(mFirstVideoPts);
                }
            }

            ChangePlayerStatus(PLAYER_PLAYING);
        }
    }

    void SuperMediaPlayer::ProcessPauseMsg()
    {
        if (mPlayStatus != PLAYER_PLAYING) {
            return;
        }

        ChangePlayerStatus(PLAYER_PAUSED);
        startRendering(false);
    }

    void SuperMediaPlayer::VideoRenderCallback(void *arg, int64_t pts, void *userData)
    {
        //   AF_LOGD("video stream render pts is %lld", pts);
        auto *pHandle = static_cast<SuperMediaPlayer *>(arg);

        if (pHandle->mCanceled) {
            return;
        }

        if ((PLAYER_PREPARED != pHandle->mPlayStatus)
                && (PLAYER_PAUSED != pHandle->mPlayStatus)
                && (PLAYER_PLAYING != pHandle->mPlayStatus)) {
            return;
        }

        MsgParam param;
        param.videoRenderedParam.pts = pts;
        param.videoRenderedParam.timeMs = af_getsteady_ms();
        param.videoRenderedParam.userData = userData;
        pHandle->putMsg(MSG_INTERNAL_VIDEO_RENDERED, param, false);
    }

    void SuperMediaPlayer::checkFirstRender()
    {
        if (!mFirstRendered) {
            mFirstRendered = true;
            updateLoopGap();
            AF_LOGI("Player NotifyFirstFrame");
            mPNotifier->NotifyFirstFrame();
        }
    }

    void SuperMediaPlayer::ProcessVideoRenderedMsg(int64_t pts, int64_t timeMs, void *picUserData)
    {
        mUtil.render(pts);
        checkFirstRender();

        if (!mSeekFlag) {
            mCurVideoPts = pts;
        }

        //AF_LOGD("video stream render pts is %lld ， mVideoChangedFirstPts = %lld ", pts, mVideoChangedFirstPts);

        if ((INT64_MIN != mVideoChangedFirstPts)
                && (pts >= mVideoChangedFirstPts)) {
            AF_LOGD("video stream changed");
            StreamInfo *info = GetCurrentStreamInfo(ST_TYPE_VIDEO);
            mPNotifier->NotifyStreamChanged(info, ST_TYPE_VIDEO);
            mVideoChangedFirstPts = INT64_MIN;
        }

        mDemuxerService->SetOption("FRAME_RENDERED", pts);

        if (mSet.bEnableVRC) {
            mPNotifier->NotifyVideoRendered(timeMs, pts);
        }

        //TODO packetGotTime
    }

    // TODO: set layout when init videoRender?

    void SuperMediaPlayer::ProcessSetDisplayMode()
    {
        if (mVideoRender) {
            mVideoRender->setScale(convertScaleMode(mSet.scaleMode));
        }
    }


    void SuperMediaPlayer::ProcessSetRotationMode()
    {
        if (mVideoRender) {
            mVideoRender->setRotate(convertRotateMode(mSet.rotateMode));
        }
    }

    void SuperMediaPlayer::ProcessSetMirrorMode()
    {
        if (mVideoRender) {
            mVideoRender->setFlip(convertMirrorMode(mSet.mirrorMode));
        }
    }

    void SuperMediaPlayer::ProcessSetVideoBackgroundColor()
    {
        if (mVideoRender) {
            mVideoRender->setBackgroundColor(mSet.mVideoBackgroundColor);
        }
    }

    void SuperMediaPlayer::ProcessSetViewMsg(void *view)
    {
        if (view == mSet.mView) {
            return;
        }

        mSet.mView = view;
        std::unique_lock<std::mutex> uMutex(mCreateMutex);

        if (mVideoRender) {
            mVideoRender->setDisPlay(view);
        }
    }

    void SuperMediaPlayer::ProcessSetDataSourceMsg(const std::string &url)
    {
        if (mPlayStatus == PLAYER_IDLE || mPlayStatus == PLAYER_STOPPED) {
            mSet.url = url;
            ChangePlayerStatus(PLAYER_INITIALZED);
        }
    }

    void SuperMediaPlayer::ChangePlayerStatus(PlayerStatus newStatus)
    {
        mOldPlayStatus = mPlayStatus;

        if (mPlayStatus != newStatus) {
            mPNotifier->NotifyPlayerStatusChanged(mPlayStatus, newStatus);
            mPlayStatus = newStatus;
            updateLoopGap();
        }
    }

    void SuperMediaPlayer::ResetSeekStatus()
    {
        mSeekPos = INT64_MIN;
        mSeekNeedCatch = false;
        updateLoopGap();
    }

    void SuperMediaPlayer::ProcessSeekToMsg(int64_t seekPos, bool bAccurate)
    {
        // seek before prepare, should keep mSeekPos
        if (mPlayStatus < PLAYER_PREPARING ||
                // if reuse player..
                mPlayStatus == PLAYER_STOPPED) {
            return;
        }

        //can seek when finished
        if ((0 >= mDuration) || (mPlayStatus >= PLAYER_STOPPED && mPlayStatus != PLAYER_COMPLETION)) {
            ResetSeekStatus();
            return;
        }

        updateLoopGap();
        //checkPosInPackQueue cache in seek
        //TODO: seek sync
        mSeekFlag = true;
        mPlayedVideoPts = INT64_MIN;
        mPlayedAudioPts = INT64_MIN;
        mSoughtVideoPos = INT64_MIN;
        mCurVideoPts = INT64_MIN;
        //flush packet queue
        mSeekInCache = SeekInCache(seekPos);

        mPNotifier->NotifySeeking(mSeekInCache);

        if (mSeekNeedCatch && !HAVE_VIDEO) {
            mSeekNeedCatch = false;
        }

        if (!mSeekInCache) {
            mBufferController.ClearPacket(BUFFER_TYPE_ALL);
            mDemuxerService->Seek(seekPos, 0, -1);
            //in case of seekpos larger than duration.
            mPNotifier->NotifyBufferPosition((seekPos <= mDuration ? seekPos : mDuration) / 1000);
            mEof = false;

            if ((mVideoChangedFirstPts != INT64_MAX)
                    && (INT64_MIN != mVideoChangedFirstPts)) {
                mVideoChangedFirstPts = seekPos;
            }
        } else {
            AF_LOGI("sought in cache");

            if (mSeekNeedCatch) {
                int64_t videoPos = mBufferController.GetKeyTimePositionBefore(BUFFER_TYPE_VIDEO, mSeekPos);

                if (videoPos < (mSeekPos - mSet.maxASeekDelta)) {
                    // first frame is far away from seek position, don't suppport accurate seek
                    mSeekNeedCatch = false;
                } else {
                    mBufferController.ClearPacketBeforeTimePos(BUFFER_TYPE_AUDIO, mSeekPos);
                }
            }

            if ((mVideoChangedFirstPts != INT64_MAX)
                    && (INT64_MIN != mVideoChangedFirstPts)
                    && (seekPos > mVideoChangedFirstPts)) {
                mVideoChangedFirstPts = seekPos;
            }
        }

        FlushVideoPath();
        FlushAudioPath();
        FlushSubtitleInfo();

        if (mSubPlayer) {
            mSubPlayer->seek(seekPos);
        }

        if ((seekPos / 1000 >= mDuration / 1000) && mDuration > 0) {
            //if eof notifySeekEndMsg
            ResetSeekStatus();
            notifySeekEndCallback();
            mSeekInCache = false;
            mEof = true;
            mSeekFlag = false;
            return;
        }

        mFirstBufferFlag = true;
        mMasterClock.setTime(seekPos);
    }

    void SuperMediaPlayer::notifySeekEndCallback()
    {
        mPNotifier->NotifySeekEnd(mSeekInCache);
    }

    void SuperMediaPlayer::ProcessMuteMsg()
    {
        if (mAudioRender == nullptr) {
            return;
        }

        mAudioRender->mute(mSet.bMute);
    }

    bool SuperMediaPlayer::IsMute() const
    {
        return mSet.bMute;
    }

    int SuperMediaPlayer::SetListener(const playerListener &Listener)
    {
        mSet.mPlayerListener = Listener;

        if (mPNotifier) {
            mPNotifier->setListener(Listener);
        }

        return 0;
    }

    void SuperMediaPlayer::SetAutoPlay(bool bAutoPlay)
    {
        mAutoPlay = bAutoPlay;
    }

    bool SuperMediaPlayer::IsAutoPlay()
    {
        return mAutoPlay;
    }

    void SuperMediaPlayer::addExtSubtitle(const char *uri)
    {
        MsgParam param;
        MsgDataSourceParam dataSourceParam = {nullptr};
        dataSourceParam.url = new string(uri ? uri : "");
        param.dataSourceParam = dataSourceParam;
        putMsg(MSG_ADD_EXT_SUBTITLE, param);
    }

    int SuperMediaPlayer::selectExtSubtitle(int index, bool bSelect)
    {
        if (!(index & EXT_STREAM_BASE)) {
            AF_LOGE("select ext subtitle error\n");
            mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_SUBTITLE_SELECT_ERROR, "Not a ext stream");
            return -1;
        }

        MsgParam param;
        MsgSelectExtSubtitleParam track = {index, bSelect};
        param.msgSelectExtSubtitleParam = track;
        putMsg(MSG_SELECT_EXT_SUBTITLE, param);
        return 0;
    }

    void SuperMediaPlayer::ProcessAddExtSubtitleMsg(const std::string &url)
    {
        lock_guard<mutex> uMutex(mCreateMutex);

        if (mSubPlayer == nullptr) {
            mSubListener = unique_ptr<mediaPlayerSubTitleListener>(new mediaPlayerSubTitleListener(*mPNotifier));
            mSubPlayer = unique_ptr<subTitlePlayer>(new subTitlePlayer(*mSubListener));
        }

        mSubPlayer->add(url);
    }

    void SuperMediaPlayer::ProcessSelectExtSubtitleMsg(int index, bool select)
    {
        lock_guard<mutex> uMutex(mCreateMutex);

        if (mSubPlayer == nullptr) {
            AF_LOGE("select ext subtitle error\n");
            mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_SUBTITLE_SELECT_ERROR, "No such subtitle stream");
            return;
        }

        int ret = mSubPlayer->select(index, select);

        if (ret < 0) {
            AF_LOGE("select ext subtitle error\n");
            mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_SUBTITLE_SELECT_ERROR, "No such subtitle stream");
        }

        if (select) {
            mSubPlayer->seek(mCurVideoPts - mMediaStartPts);
        }
    }

    void SuperMediaPlayer::ProcessVideoCleanFrameMsg()
    {
        while (!mVideoFrameQue.empty()) {
            int64_t pts = mVideoFrameQue.front()->getInfo().pts;
            ProcessVideoRenderedMsg(pts, af_getsteady_ms(), nullptr);
            mVideoFrameQue.pop();
        }

        if (mVideoRender) {
            unique_ptr<IAFFrame> frame = nullptr;
            mVideoRender->renderFrame(frame);
        }

        mPlayedVideoPts = INT64_MIN;
        mCurVideoPts = INT64_MIN;
        videoDecoderFull = false;
        mVideoPtsRevert = false;
        dropLateVideoFrames = true;
    }

    void SuperMediaPlayer::ProcessVideoHoldMsg(bool hold)
    {
        if (mVideoDecoder) {
            mVideoDecoder->holdOn(hold);

            if (!hold) {
                int size = mVideoDecoder->getRecoverQueueSize();

                if (size > mSet.maxVideoRecoverSize) {
                    string des = "video decoder recover size too large:" + AfString::to_string(size);
                    mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_DECODER_RECOVER_SIZE, des.c_str());
                }
            }
        }
    }

    void SuperMediaPlayer::ProcessSetSpeed(float speed)
    {
        if (!CicadaUtils::isEqual(mSet.rate, speed)) {
            if (HAVE_AUDIO) {
                if (mAudioRender != nullptr) {
                    mAudioRender->setSpeed(speed);
                }
            }

            if (mVideoRender) {
                mVideoRender->setSpeed(speed);
            }

            mSet.rate = speed;
            updateLoopGap();
            mMasterClock.SetScale(speed);
        }
    }

    void SuperMediaPlayer::startRendering(bool start)
    {
        if (start == mBRendingStart) {
            return;
        }

        mBRendingStart = start;

        if (start) {
            mMasterClock.start();
        } else {
            mMasterClock.pause();
        }

        if (mAudioRender) {
            mAudioRender->pause(!start);
        }
    }
    void SuperMediaPlayer::SetOnRenderCallBack(onRenderFrame cb, void *userData)
    {
        mFrameCb = cb;
        mFrameCbUserData = userData;
    }
}//namespace Cicada
