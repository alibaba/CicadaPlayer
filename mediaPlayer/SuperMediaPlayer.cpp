#define LOG_TAG "ApsaraPlayerService"

#include "SuperMediaPlayer.h"
#include "media_player_error_def.h"
#include "media_player_error_map.h"
#include "utils/CicadaJSON.h"
#include "utils/CicadaUtils.h"
#include <cassert>
#include <cinttypes>
#include <codec/avcodecDecoder.h>
#include <codec/decoderFactory.h>
#include <data_source/dataSourcePrototype.h>
#include <demuxer/IDemuxer.h>
#include <render/renderFactory.h>
#include <utils/AFMediaType.h>
#include <utils/af_string.h>
#include <utils/err.h>
#include <utils/errors/framework_error.h>
#include <utils/ffmpeg_utils.h>
#include <utils/file/FileUtils.h>
#include <utils/frame_work_log.h>
#include <utils/oscl/oscl_utils.h>
#include <utils/property.h>
#include <utils/timer.h>


#ifdef __APPLE__

#include <TargetConditionals.h>
#include <codec/Apple/AppleVideoToolBox.h>
#include <render/audio/Apple/AFAudioSessionWrapper.h>

#endif

#define PTS_DISCONTINUE_DELTA (20 * 1000 * 1000)
#define VIDEO_PICTURE_MAX_CACHE_SIZE 2

static int MAX_DECODE_ERROR_FRAME = 1000;

#define PTS_REVERTING (mVideoPtsRevert != mAudioPtsRevert)

using namespace Cicada;
SuperMediaPlayer SuperMediaPlayer::se(1);

static MsgParam dummyMsg{{nullptr}};

const int64_t SuperMediaPlayer::SEEK_ACCURATE_MAX = 11 * 1000 * 1000;

#define HAVE_VIDEO (mCurrentVideoIndex >= 0)
#define HAVE_AUDIO (mCurrentAudioIndex >= 0)
#define HAVE_SUBTITLE (mCurrentSubtitleIndex >= 0)

SuperMediaPlayer::SuperMediaPlayer()
{
    AF_LOGD("SuperMediaPlayer()");

    mSet = static_cast<unique_ptr<player_type_set>>(new player_type_set());
    mBufferController = static_cast<unique_ptr<BufferController>>(new BufferController());
    mUtil = static_cast<unique_ptr<MediaPlayerUtil>>(new MediaPlayerUtil());
    mMsgCtrlListener = static_cast<unique_ptr<SMPMessageControllerListener>>(new SMPMessageControllerListener(*this));
    mMessageControl = static_cast<unique_ptr<PlayerMessageControl>>(new PlayerMessageControl(*mMsgCtrlListener));
    mAudioRenderCB = static_cast<unique_ptr<ApsaraAudioRenderCallback>>(new ApsaraAudioRenderCallback(*this));
    mVideoRenderListener = static_cast<unique_ptr<ApsaraVideoRenderListener>>(new ApsaraVideoRenderListener(*this));
    mApsaraThread = static_cast<unique_ptr<afThread>>(new afThread([this]() -> int { return this->mainService(); }, LOG_TAG));
    mSourceListener = static_cast<unique_ptr<SuperMediaPlayerDataSourceListener>>(new SuperMediaPlayerDataSourceListener(*this));
    mDcaManager = static_cast<unique_ptr<SMP_DCAManager>>(new SMP_DCAManager(*this));
    mAVDeviceManager = static_cast<unique_ptr<SMPAVDeviceManager>>(new SMPAVDeviceManager());
    mRecorderSet = static_cast<unique_ptr<SMPRecorderSet>>(new SMPRecorderSet());

    mPNotifier = new PlayerNotifier();
    Reset();
    mTimerInterval = 500;
}

SuperMediaPlayer::~SuperMediaPlayer()
{
    if (mIsDummy) {
        return;
    }
    Stop();
    AF_LOGD("SuperMediaPlayer");
    mCanceled = true;
    mPlayerCondition.notify_one();
    mApsaraThread->stop();
    mSubPlayer = nullptr;
    mSubListener = nullptr;
    // delete mPNotifier after mPMainThread, to avoid be using
    delete mPNotifier;
    mPNotifier = nullptr;
    mMessageControl = nullptr;
}

void SuperMediaPlayer::putMsg(PlayMsgType type, const MsgParam &param, bool trigger)
{
    mMessageControl->putMsg(type, param);

    if (trigger) {
        mPlayerCondition.notify_one();
    }
}

void SuperMediaPlayer::SetView(void *view)
{
    mMsgCtrlListener->ProcessSetViewMsg(view);
}

int64_t SuperMediaPlayer::GetMasterClockPts()
{
    return mMasterClock.GetTime();
}

void SuperMediaPlayer::setBitStreamCb(readCB read, seekCB seek, void *arg)
{
    MsgParam param;
    MsgBitStreamParam msgBitStreamParam = {nullptr};
    msgBitStreamParam.read = read;
    msgBitStreamParam.seek = seek;
    msgBitStreamParam.arg = arg;
    param.msgBitStreamParam = msgBitStreamParam;
    putMsg(MSG_SET_BITSTREAM, param);
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
    mApsaraThread->start();
}

void SuperMediaPlayer::CaptureScreen()
{
    std::lock_guard<std::mutex> uMutex(mCreateMutex);

    if (mAVDeviceManager->getVideoRender()) {
        mAVDeviceManager->getVideoRender()->captureScreen([this](uint8_t *data, int width, int height) {
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
    mSet->mVolume = volume;

    if (mSet->mVolume < 0) {
        mSet->mVolume = 0;
    } else if (mSet->mVolume > 1.0) {
        AF_LOGW("volume >1.0");
    }

    mAVDeviceManager->setVolume(mSet->mVolume);
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
void SuperMediaPlayer::Mute(bool bMute)
{
    if (bMute == mSet->bMute) {
        return;
    }

    mSet->bMute = bMute;
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
    std::deque<StreamInfo *> &streamInfoQueue = mMediaInfo.mStreamInfoQueue;
    for (auto &it : streamInfoQueue) {
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
    if ((afThread::THREAD_STATUS_RUNNING != mApsaraThread->getStatus()) &&
        ((mPlayStatus == PLAYER_IDLE) || (mPlayStatus == PLAYER_STOPPED))) {
        return 0;
    }

    std::unique_lock<std::mutex> uMutex(mPlayerMutex);
    AF_LOGI("Player ReadPacket Stop");
    int64_t t1 = af_getsteady_ms();
    AF_TRACE;
    waitingForStart = false;
    mCanceled = true;
    mPNotifier->Clean();
    mPNotifier->Enable(false);
    Interrupt(true);
    mPlayerCondition.notify_one();
    mApsaraThread->pause();
    mAVDeviceManager->invalidDevices(SMPAVDeviceManager::DEVICE_TYPE_AUDIO | SMPAVDeviceManager::DEVICE_TYPE_VIDEO);
    mPlayStatus = PLAYER_STOPPED;
    //        ChangePlayerStatus(PLAYER_STOPPED);
    mBufferController->ClearPacket(BUFFER_TYPE_AV);

    AF_TRACE;
    FlushAudioPath();

    AF_TRACE;
    mBRendingStart = false;
    AF_TRACE;
    FlushVideoPath();
    // clear the message queue after flash video render
    mMessageControl->clear();
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
    }

    if (mDataSource) {
        mDataSource->Close();
        std::lock_guard<std::mutex> uMutex(mCreateMutex);
        delete mDataSource;
        mDataSource = nullptr;
    }

    if (mAVDeviceManager->getVideoRender()) {
        // lock mAppStatusMutex before mCreateMutex
        std::lock_guard<std::mutex> lock(mAppStatusMutex);

        // for iOS, don't delete render in background, and we should reuse it later.
        if (APP_BACKGROUND != mAppStatus) {
            std::lock_guard<std::mutex> uMutex(mCreateMutex);

            if (mSet->clearShowWhenStop) {
                mAVDeviceManager->getVideoRender()->clearScreen();
            }
        }
    }

    delete mVideoParser;
    mVideoParser = nullptr;
    {
        std::lock_guard<std::mutex> uMutex(mCreateMutex);
        std::deque<StreamInfo *> &streamInfoQueue = mMediaInfo.mStreamInfoQueue;
        for (StreamInfo *info : streamInfoQueue) {
            releaseStreamInfo(info);
        }
        streamInfoQueue.clear();
        mMediaInfo.totalBitrate = 0;
    }
    mBufferController->ClearPacket(BUFFER_TYPE_SUBTITLE);
    Reset();

    mRecorderSet->reset();

    AF_LOGD("stop spend time is %lld", af_getsteady_ms() - t1);
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
        mSet->refer = referer;
    }
}

void SuperMediaPlayer::SetUserAgent(const char *userAgent)
{
    if (userAgent) {
        mSet->userAgent = userAgent;
    }
}

void SuperMediaPlayer::SetTimeout(int timeout)
{
    mSet->timeout_ms = timeout;
}

void SuperMediaPlayer::SetDropBufferThreshold(int dropValue)
{
    mSet->RTMaxDelayTime = dropValue * 1000;
}

void SuperMediaPlayer::SetLooping(bool looping)
{
    mSet->bLooping = looping;
}

bool SuperMediaPlayer::isLooping()
{
    return mSet->bLooping;
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
            mSet->startBufferDuration = duration * 1000;
        }
    } else if (theKey == "RTMaxDelayTime") {
        duration = atoi(value);

        if (duration > 0) {
            mSet->RTMaxDelayTime = duration * 1000;
            mSet->mOptions.set(theKey, std::to_string(mSet->RTMaxDelayTime), options::REPLACE);
        }
    } else if (theKey == "highLevelBufferDuration") {
        duration = atoi(value);

        if (duration > 0) {
            mSet->highLevelBufferDuration = duration * 1000;
            mSet->mOptions.set(theKey, std::to_string(mSet->highLevelBufferDuration), options::REPLACE);
        }
    } else if (theKey == "http_proxy") {
        mSet->http_proxy = value;
    } else if (theKey == "maxBufferDuration") {
        duration = atoi(value);

        if (duration > 0) {
            mSet->maxBufferDuration = int64_t(duration) * 1000;
            mSet->mOptions.set(theKey, std::to_string(mSet->maxBufferDuration), options::REPLACE);
        }
    } else if (theKey == "LowLatency") {
        mSet->bLowLatency = (bool) atoi(value);
    } else if (theKey == "ClearShowWhenStop") {
        int clearShowWhenStop = atoi(value);
        mSet->clearShowWhenStop = (bool) clearShowWhenStop;
    } else if (theKey == "enableVideoTunnelRender") {
        mSet->bEnableTunnelRender = (atoi(value) != 0);
    } else if (theKey == "disableAudio") {
        mSet->bDisableAudio = (atoi(value) != 0);
    } else if (theKey == "disableVideo") {
        mSet->bDisableVideo = (atoi(value) != 0);
    } else if (theKey == "timerInterval") {
        mTimerInterval = atoi(value);
    } else if (theKey == "Analytics.ReportID") {
        if (nullptr == value) {
            return -1;
        }

        int64_t eventReportID = atoll(value);
        mSet->AnalyticsID = eventReportID;
    } else if (theKey == "bandWidth") {
        mSet->mDefaultBandWidth = atoi(value);
    } else if (theKey == "description") {
        mSet->mOptions.set(theKey, value, options::REPLACE);
        return 0;
    } else if (theKey == "enableVRC") {
        mSet->bEnableVRC = (atoi(value) != 0);
    } else if (theKey == "maxAccurateSeekDelta") {
        mSet->maxASeekDelta = atoi(value) * 1000;
    } else if (theKey == "maxVideoRecoverSize") {
        mSet->maxVideoRecoverSize = atoi(value);
    } else if (theKey == "surfaceChanged") {
        std::lock_guard<std::mutex> uMutex(mCreateMutex);

        if (mAVDeviceManager->isVideoRenderValid()) {
            mAVDeviceManager->getVideoRender()->surfaceChanged();
        }
    } else if (theKey == "streamTypes") {
        uint64_t flags = atoll(value);
        mSet->bDisableAudio = mSet->bDisableVideo = true;
        if (flags & VIDEO_FLAG) {
            mSet->bDisableVideo = false;
        }
        if (flags & AUDIO_FLAG) {
            mSet->bDisableAudio = false;
        }
    } else if (theKey == "IPResolveType") {
        uint64_t type = atoll(value);
        mSet->mIpType = static_cast<IpResolveType>(type);
    } else if (theKey == "fastStart") {
        mSet->mFastStart = atol(value) != 0;
    } else if (theKey == "pixelBufferOutputFormat") {
        mSet->pixelBufferOutputFormat = atol(value);
    } else if (theKey == "liveStartIndex") {
        mSet->mOptions.set(theKey, value, options::REPLACE);
    } else if (theKey == "DRMMagicKey") {
        mSet->drmMagicKey = value;
    } else if (theKey == "sessionId") {
        mSet->sessionId = value;

        std::lock_guard<std::mutex> uMutex(mCreateMutex);
        if (mDemuxerService != nullptr && mDemuxerService->getDemuxerHandle()) {
            mDemuxerService->getDemuxerHandle()->SetOption("sessionId", mSet->sessionId);
        }
    } else if (theKey == "networkRetryCount") {
        mSet->netWorkRetryCount = (int) atol(value);
    } else if (theKey == "maxBackwardBufferDuration") {
        mBufferController->SetMaxBackwardDuration(BUFFER_TYPE_ALL, atoll(value) * 1000);
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
        snprintf(value, MAX_OPT_VALUE_LENGTH, "%" PRId64 "", mSet->maxBufferDuration);
    } else if (theKey == "mediaStreamSize") {
        int64_t size = -1;
        std::unique_lock<std::mutex> uMutex(mCreateMutex);

        if (mDataSource && mDemuxerService) {
            if (!mDemuxerService->isPlayList()) {
                size = mDataSource->Seek(0, SEEK_SIZE);
            }
        } else if (mBSSeekCb) {
            size = mBSSeekCb(mBSCbArg, 0, SEEK_SIZE);
        }

        snprintf(value, MAX_OPT_VALUE_LENGTH, "%" PRId64 "", size);
    } else if (theKey == "description") {
        sprintf(value, "%s", mSet->mOptions.get("description").c_str());
    } else if (theKey == "descriptionLen") {
        snprintf(value, MAX_OPT_VALUE_LENGTH, "%lu", static_cast<unsigned long>(mSet->mOptions.get("description").length()));
    } else if (theKey == "renderFps") {
        float renderFps = GetVideoRenderFps();
        snprintf(value, MAX_OPT_VALUE_LENGTH, "%f", renderFps);
    }
}

void SuperMediaPlayer::NotifyPosition(int64_t position)
{
    mPNotifier->NotifyPosition(position / 1000);
}

int64_t SuperMediaPlayer::getCurrentPosition()
{
    if (isSeeking()) {
        return mSeekPos;
    }

    mCurrentPos = mCurrentPos.load() < 0 ? 0 : mCurrentPos.load();

    if (mDuration > 0) {
        mCurrentPos = mCurrentPos.load() <= mDuration ? mCurrentPos.load() : mDuration;
    }

    return mCurrentPos;
}

void SuperMediaPlayer::SetScaleMode(ScaleMode mode)
{
    if (mode == mSet->scaleMode) {
        return;
    }

    mSet->scaleMode = static_cast<ScaleMode>(mode);
    this->putMsg(MSG_SET_DISPLAY_MODE, dummyMsg);
}

void SuperMediaPlayer::SetRotateMode(RotateMode mode)
{
    if (mode == mSet->rotateMode) {
        return;
    }

    mSet->rotateMode = static_cast<RotateMode>(mode);
    this->putMsg(MSG_SET_ROTATE_MODE, dummyMsg);
}

RotateMode SuperMediaPlayer::GetRotateMode()
{
    return mSet->rotateMode;
}

void SuperMediaPlayer::SetMirrorMode(MirrorMode mode)
{
    if (mode == mSet->mirrorMode) {
        return;
    }

    mSet->mirrorMode = static_cast<MirrorMode>(mode);
    this->putMsg(MSG_SET_MIRROR_MODE, dummyMsg);
}

void SuperMediaPlayer::SetVideoBackgroundColor(uint32_t color)
{
    if (color == mSet->mVideoBackgroundColor) {
        return;
    }

    mSet->mVideoBackgroundColor = color;
    this->putMsg(MSG_SET_VIDEO_BACKGROUND_COLOR, dummyMsg);
}

MirrorMode SuperMediaPlayer::GetMirrorMode()
{
    return mSet->mirrorMode;
}

ScaleMode SuperMediaPlayer::GetScaleMode()
{
    return mSet->scaleMode;
}

int64_t SuperMediaPlayer::GetBufferPosition()
{
    return mBufferPosition / 1000;
}

int64_t SuperMediaPlayer::GetDuration() const
{
    if (mDuration != INT64_MIN) {
        return mDuration / 1000;
    }
    return -1;
}

// TODO: change name to EnableHwDecode
void SuperMediaPlayer::SetDecoderType(DecoderType type)
{
    mSet->bEnableHwVideoDecode = (type == DT_HARDWARE);
}

void SuperMediaPlayer::AddCustomHttpHeader(const char *header)
{
    for (auto &item : mSet->customHeaders) {
        if (item == header) {
            return;
        }
    }

    mSet->customHeaders.emplace_back(header);
}

void SuperMediaPlayer::RemoveAllCustomHttpHeader()
{
    mSet->customHeaders.clear();
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
    return mSet->rate;
}

DecoderType SuperMediaPlayer::GetDecoderType()
{
    std::lock_guard<std::mutex> uMutex(mCreateMutex);

    if (mAVDeviceManager->getVideoDecoderFlags() & DECFLAG_HW) {
        return DT_HARDWARE;
    }

    return DT_SOFTWARE;
}

PlayerStatus SuperMediaPlayer::GetPlayerStatus() const
{
    return mPlayStatus;
}

float SuperMediaPlayer::GetVolume() const
{
    return mSet->mVolume;
}

int64_t SuperMediaPlayer::GetPropertyInt(PropertyKey key)
{
    switch (key) {
        case PROPERTY_KEY_VIDEO_BUFFER_LEN: {
            int64_t duration = mBufferController->GetPacketDuration(BUFFER_TYPE_VIDEO);

            if (duration < 0) {
                duration = mBufferController->GetPacketLastPTS(BUFFER_TYPE_VIDEO) - mBufferController->GetPacketPts(BUFFER_TYPE_VIDEO);
            }

            return duration;
        }

        case PROPERTY_KEY_REMAIN_LIVE_SEG:
            return mRemainLiveSegment;

        case PROPERTY_KEY_NETWORK_IS_CONNECTED:
            return mSourceListener->isConnected();

        case PROPERTY_KEY_RE_BUFFERING:
            return mBufferingFlag;

        case PROPERTY_KEY_DOWNLOAD_COMPLETED:
            return mEof;

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
                std::deque<StreamInfo *> &streamInfoQueue = mMediaInfo.mStreamInfoQueue;
                MediaPlayerUtil::getPropertyJSONStr("responseInfo", array, false, streamInfoQueue, mDemuxerService.get());
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
                std::deque<StreamInfo *> &streamInfoQueue = mMediaInfo.mStreamInfoQueue;
                MediaPlayerUtil::getPropertyJSONStr("openJsonInfo", array, true, streamInfoQueue, mDemuxerService.get());
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
                std::deque<StreamInfo *> &streamInfoQueue = mMediaInfo.mStreamInfoQueue;
                MediaPlayerUtil::getPropertyJSONStr("probeInfo", array, false, streamInfoQueue, mDemuxerService.get());
            } else {
                CicadaJSONItem item(mDemuxerService->GetProperty(0, "probeInfo"));
                item.addValue("type", "video");
                array.addJSON(item);
            }

            return array.printJSON();
        }

        case PROPERTY_KEY_DELAY_INFO: {
            std::lock_guard<std::mutex> uMutex(mCreateMutex);
            if (nullptr != mDemuxerService) {
                string ret = mDemuxerService->GetProperty(0, "delayInfo");
                return ret;
            }

            return "";
        }
        case PROPERTY_KEY_PLAY_CONFIG: {
            CicadaJSONItem item{};
            item.addValue("http_proxy", mSet->http_proxy);
            item.addValue("refer", mSet->refer);
            item.addValue("timeout_ms", (int) mSet->timeout_ms);
            item.addValue("RTMaxDelayTime", (int) mSet->RTMaxDelayTime);
            item.addValue("startBufferDuration", (int) mSet->startBufferDuration);
            item.addValue("highLevelBufferDuration", (int) mSet->highLevelBufferDuration);
            item.addValue("maxBufferDuration", (int) mSet->maxBufferDuration);
            return item.printJSON();
        }
        case PROPERTY_KEY_DECODE_INFO: {
            CicadaJSONArray decodeInfos{};
            if (HAVE_AUDIO) {
                CicadaJSONItem audioDecodeInfo{};
                audioDecodeInfo.addValue("type", "audio");
                audioDecodeInfo.addValue("createDecodeCost", (int) mRecorderSet->createAudioDecoderCostMs);
                audioDecodeInfo.addValue("decodeFirstCost", (int) mRecorderSet->decodeFirstAudioFrameInfo.getDecodeFirstFrameCost());
                audioDecodeInfo.addValue("firstSize", (int) mRecorderSet->decodeFirstAudioFrameInfo.firstPacketSize);
                audioDecodeInfo.addValue("firstPts", (double) mRecorderSet->decodeFirstAudioFrameInfo.firstPacketPts);
                decodeInfos.addJSON(audioDecodeInfo);
            }
            if (HAVE_VIDEO) {
                CicadaJSONItem videoDecodeInfo{};
                videoDecodeInfo.addValue("type", "video");
                videoDecodeInfo.addValue("createDecodeCost", (int) mRecorderSet->createVideoDecoderCostMs);
                videoDecodeInfo.addValue("decodeFirstCost", (int) mRecorderSet->decodeFirstVideoFrameInfo.getDecodeFirstFrameCost());
                videoDecodeInfo.addValue("firstSize", (int) mRecorderSet->decodeFirstVideoFrameInfo.firstPacketSize);
                videoDecodeInfo.addValue("firstPts", (double) mRecorderSet->decodeFirstVideoFrameInfo.firstPacketPts);
                decodeInfos.addJSON(videoDecodeInfo);
            }

            return decodeInfos.printJSON();
        }
        case PROPERTY_KEY_HLS_KEY_URL: {
            std::lock_guard<std::mutex> uMutex(mCreateMutex);
            if (nullptr != mDemuxerService) {
                return mDemuxerService->GetProperty(0, "keyUrl");
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
    mSourceListener->enableRetry();
    std::lock_guard<std::mutex> uMutex(mCreateMutex);
    if (mDemuxerService && mDemuxerService->getDemuxerHandle()) {
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
    switch (mode) {
        case MirrorMode::MIRROR_MODE_HORIZONTAL:
            return IVideoRender::Flip::Flip_Horizontal;
        case MirrorMode::MIRROR_MODE_VERTICAL:
            return IVideoRender::Flip::Flip_Vertical;
        default:
            return IVideoRender::Flip::Flip_None;
    }
}

float SuperMediaPlayer::GetVideoRenderFps()
{
    if (mAVDeviceManager->isVideoRenderValid()) {
        return mAVDeviceManager->getVideoRender()->getRenderFPS();
    }

    return mUtil->getVideoRenderFps();
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

int SuperMediaPlayer::updateLoopGap()
{
    switch (mPlayStatus.load()) {
        case PLAYER_PREPARINIT:
        case PLAYER_PREPARING:
        case PLAYER_PREPARED:
            return 3;

        case PLAYER_PLAYING:
            if (!mFirstRendered) {
                return 3;
            } else if (HAVE_VIDEO) {
                if (mCurrentVideoMeta) {
                    // the loop gap can't too low
                    int fps = std::max(25, (int) (mCurrentVideoMeta->operator Stream_meta *()->avg_fps));
                    if (mVideoInterlaced == InterlacedType_YES) {
                        fps *= 2;
                    }
                    return 1000 / int((float) fps * mSet->rate * 1.5);
                }
            }
            return 1000 / int(50 * mSet->rate);

        default:
            return 40;
    }
}

int SuperMediaPlayer::mainService()
{
    int64_t curTime = af_gettime_relative();
    mUtil->notifyPlayerLoop(curTime);
    sendDCAMessage();

    if (mMessageControl->empty() || (0 == mMessageControl->processMsg())) {
        ProcessVideoLoop();
        int loopGap = updateLoopGap();
        int64_t use = (af_gettime_relative() - curTime) / 1000;
        int64_t needWait = loopGap - use;
        // AF_LOGD("use :%lld, needWait:%lld", use, needWait);

        if (needWait <= 0) {
            if (loopGap < 5) {
                needWait = 2;
            } else {
                return 0;
            }
        }
        if (mVideoCatchingUp) {
            return 0;
        }

        std::unique_lock<std::mutex> uMutex(mSleepMutex);
        mPlayerCondition.wait_for(uMutex, std::chrono::milliseconds(needWait), [this]() { return this->mCanceled.load(); });
    }

    return 0;
}

void SuperMediaPlayer::sendDCAMessage()
{
    string event = mDcaManager->getEvent();
    while (!event.empty()) {
        mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_DIRECT_COMPONENT_MSG, event.c_str());
        event = mDcaManager->getEvent();
    }
}

void SuperMediaPlayer::ProcessVideoLoop()
{
    int64_t curTime = af_gettime_relative() / 1000;
    {
#ifndef NDEBUG
        int streamIds[] = {mCurrentVideoIndex,           mCurrentAudioIndex,           mCurrentSubtitleIndex,
                           mWillChangedVideoStreamIndex, mWillChangedAudioStreamIndex, mWillChangedSubtitleStreamIndex};
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

    if ((PLAYER_COMPLETION != mPlayStatus && (mPlayStatus < PLAYER_PREPARING || mPlayStatus > PLAYER_PAUSED)) ||
        nullptr == mDemuxerService) {// not working
        if (curTime - mTimerLatestTime > mTimerInterval) {
            OnTimer(curTime);
            mTimerLatestTime = curTime;
        }

        return;
    }
    doReadPacket();
    doDeCode();

    // audio render will create after get a frame from decoder
    setUpAVPath();

    if (!DoCheckBufferPass()) {
        return;
    }

    if (!mBRendingStart && mPlayStatus == PLAYER_PLAYING && !mBufferingFlag) {
        if ((mEof && (!HAVE_AUDIO || mAVDeviceManager->isAudioRenderValid()) &&
             (!HAVE_VIDEO || mAVDeviceManager->isVideoRenderValid())) ||// render out the cache frame in renders
            ((!HAVE_VIDEO || !mVideoFrameQue.empty() || (APP_BACKGROUND == mAppStatus)) && (!HAVE_AUDIO || !mAudioFrameQue.empty()))) {
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
    int64_t cur_buffer_duration = getPlayerBufferDuration(false, false);
    //100s
    mUtil->notifyRead(MediaPlayerUtil::readEvent_Loop, 0);

    if (mEof) {
        return;
    }

    //demuxer read
    int64_t read_start_time = af_gettime_relative();
    int timeout = 10000;
    mem_info info{};
    int checkStep = 0;

    while (true) {
        // once buffer is full, we will try to read again if buffer consume more then BufferGap
        if (mBufferIsFull) {
            static const int BufferGap = 1000 * 1000;

            if ((mSet->maxBufferDuration > 2 * BufferGap) && (cur_buffer_duration > mSet->maxBufferDuration - BufferGap) &&
                getPlayerBufferDuration(false, true) > 0) {
                break;
            }
        }

        if (cur_buffer_duration > mSet->maxBufferDuration &&
            getPlayerBufferDuration(false, true) > 0// we need readout the buffer in demuxer when no buffer in player
        ) {
            mBufferIsFull = true;
            break;
        }

        mBufferIsFull = false;

        if ((0 >= checkStep--) && (cur_buffer_duration > 1000 * 1000) && (AFGetSystemMemInfo(&info) >= 0)) {
            //AF_LOGD("system_availableram is %" PRIu64 "",info.system_availableram);
            if (info.system_availableram > 2 * mSet->lowMemSize) {
                checkStep = (int) (info.system_availableram / (5 * 1024 * 1024));
#ifdef ANDROID
            } else if (0 && info.system_availableram < mSet->lowMemSize) {
#else
            } else if (info.system_availableram < mSet->lowMemSize) {
#endif
                AF_LOGW("low memory...");

                if (!mLowMem) {
                    mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_SYSTEM_LOW_MEMORY, "App Low memory");
                }

                mLowMem = true;

                if (mSet->highLevelBufferDuration > 800 * 1000) {
                    mSet->highLevelBufferDuration = 800 * 1000;
                }

                if (mSet->startBufferDuration > 800 * 1000) {
                    mSet->startBufferDuration = 800 * 1000;
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

            mUtil->notifyRead(MediaPlayerUtil::readEvent_Again, 0);
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
            AF_LOGE("read error %s\n", framework_err2_string(ret));
            NotifyError(ret);
            break;
        } else if (ret < 0) {
            if (!mBufferingFlag && mPlayStatus >= PLAYER_PREPARED) {
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
            mUtil->notifyRead(MediaPlayerUtil::readEvent_timeOut, 0);
            //                    mMsgProcessTime = 0;
            break;
        }

        cur_buffer_duration = getPlayerBufferDuration(false, false);
        //                if(getPlayerBufferDuration(true) > mSet->maxBufferDuration * 2){
        //                    AF_LOGE("buffer stuffed\n");
        //                    mPNotifier->NotifyError(MEDIA_PLAYER_ERROR_BUFFER_STUFFED,"buffer stuffed");
        //                    break;
        //                }
    }
}

void SuperMediaPlayer::OnDemuxerCallback(const std::string &key, const std::string &value)
{}

bool SuperMediaPlayer::DoCheckBufferPass()
{
    int64_t cur_buffer_duration = getPlayerBufferDuration(false, false);
    int64_t HighBufferDur = mSet->highLevelBufferDuration;

    if (mFirstBufferFlag && !mEof) {
        HighBufferDur = mSet->startBufferDuration;

        //clean late audio data
        if (cur_buffer_duration > HighBufferDur && HAVE_VIDEO && HAVE_AUDIO) {
            if (mSoughtVideoPos > 0) {
                AF_LOGW("clean late audio data before %lld", mSoughtVideoPos);
                mBufferController->ClearPacketBeforeTimePos(BUFFER_TYPE_AUDIO, mSoughtVideoPos);
                int64_t pts = mBufferController->GetPacketPts(BUFFER_TYPE_AUDIO);

                if (mRemovedFirstAudioPts == INT64_MIN) {
                    mRemovedFirstAudioPts = pts;

                    if (mFirstAudioPts == INT64_MIN) {
                        mFirstAudioPts = pts - mFirstSeekStartTime;
                    }
                }

                cur_buffer_duration = getPlayerBufferDuration(false, false);

                if (cur_buffer_duration < HighBufferDur) {
                    return false;
                }
            }
        }
    }

    bool isTimeSync = false;
    if (mDemuxerService != nullptr) {
        isTimeSync = mDemuxerService->isWallclockTimeSyncStream(mCurrentVideoIndex);
    }

    if (mPlayStatus == PLAYER_PREPARING) {

        if (isTimeSync) {
            int currentStreamIndex = mCurrentAudioIndex;
            if (!HAVE_AUDIO) {
                currentStreamIndex = mCurrentVideoIndex;
            }
            int64_t durationToStart = mDemuxerService->getDurationToStartStream(currentStreamIndex);
            if (durationToStart < 0) {
                mPNotifier->NotifyPosition(durationToStart / 1000);
                return false;
            }
        }

        if ((cur_buffer_duration >= HighBufferDur &&
             (!HAVE_VIDEO || !mAVDeviceManager->isDecoderValid(SMPAVDeviceManager::DEVICE_TYPE_VIDEO) || videoDecoderFull ||
              APP_BACKGROUND == mAppStatus || !mSet->mFastStart)) ||
            (mEof)) {
            if (mEof && getPlayerBufferDuration(true, false) <= 0) {
                // If player don`t get any packets when read eof
                if (mSeekPos > 0) {
                    //If caused by before prepare seeked, treat as play completed.
                    mPNotifier->NotifyLoading(loading_event_end, 0);
                    playCompleted();
                } else {
                    ChangePlayerStatus(PLAYER_ERROR);
                    mPNotifier->NotifyError(MEDIA_PLAYER_ERROR_DEMUXER_OPENSTREAM, "open stream failed");
                }
            } else {
                ChangePlayerStatus(PLAYER_PREPARED);
                AF_LOGD("PLAYER_PREPARED");
                AF_LOGD("prepare use %lld ms\n", (af_gettime_relative() - mPrepareStartTime) / 1000);
                notifyPreparedCallback();

                if (mFirstBufferFlag) {
                    mFirstBufferFlag = false;
                }
            }
        } else {

            int64_t duration_v = -1;
            int64_t duration_a = -1;

            if (HAVE_VIDEO) {
                duration_v = mBufferController->GetPacketDuration(BUFFER_TYPE_VIDEO);
            }
            if (HAVE_AUDIO) {
                duration_a = mBufferController->GetPacketDuration(BUFFER_TYPE_AUDIO);
            }

            /**
             * if meta has audio and video stream infos , but after read 2 minutes duration ,
             * one of streams still has no buffer duration , close it to avoid read all packets.
             */
            if (std::min(duration_v, duration_a) == 0 && std::max(duration_v, duration_a) > 2 * 60 * 1000000) {
                if (duration_v > duration_a) {
                    if (mAVDeviceManager->isDecoderValid(SMPAVDeviceManager::DEVICE_TYPE_AUDIO)) {
                        mDemuxerService->CloseStream(mCurrentAudioIndex);
                        mCurrentAudioIndex = -1;
                        mMasterClock.setReferenceClock(nullptr, nullptr);
                        mAudioFrameQue.clear();
                        mBufferController->ClearPacket(BUFFER_TYPE_AUDIO);
                        AF_LOGW("close audio stream");
                    }
                } else {
                    if (mAVDeviceManager->isDecoderValid(SMPAVDeviceManager::DEVICE_TYPE_VIDEO)) {
                        closeVideo();
                    }
                }
            }
        }
    }

    //check buffering empty
    if (!mEof && cur_buffer_duration <= 0 && !mBufferingFlag && (mPlayStatus == PLAYER_PLAYING || mPlayStatus == PLAYER_PAUSED)) {
        mBufferingFlag = true;
        mPNotifier->NotifyLoading(loading_event_start, 0);
        AF_LOGD("loading start");
        mLoadingProcess = 0;
        mTimeoutStartTime = INT64_MIN;
        mMasterClock.pause();
        mAVDeviceManager->pauseAudioRender(true);
        return false;
    }

    //AF_LOGD("current duration is %lld,video duration is %lld,audio duration is %lld", cur_buffer_duration
    //	,mBufferController->GetPacketDuration(BUFFER_TYPE_VIDEO), mBufferController->GetPacketDuration(BUFFER_TYPE_AUDIO));
    bool isRealTime = false;
    if (mDemuxerService != nullptr) {
        isRealTime = mDemuxerService->isRealTimeStream(mCurrentVideoIndex);
    }

    while (isRealTime && mSet->RTMaxDelayTime > 0) {
        if (!HAVE_AUDIO) {
            int64_t maxBufferDuration = getPlayerBufferDuration(true, false);

            if (maxBufferDuration > mSet->RTMaxDelayTime + 1000 * 1000 * 5) {
                int64_t lastKeyPos = mBufferController->GetPacketLastKeyTimePos(BUFFER_TYPE_VIDEO);
                mBufferController->ClearPacketBeforeTimePos(BUFFER_TYPE_VIDEO, lastKeyPos);
                break;
            }

            LiveCatchUp(maxBufferDuration);
            break;
        }

        int64_t maxBufferDuration = getPlayerBufferDuration(true, false);

        if (maxBufferDuration > mSet->RTMaxDelayTime + 1000 * 1000 * 5) {
            //drop frame
            int64_t lastVideoPos = mBufferController->GetPacketLastTimePos(BUFFER_TYPE_VIDEO);
            int64_t lastAudioPos = mBufferController->GetPacketLastTimePos(BUFFER_TYPE_AUDIO);
            int64_t lastAudioPts = mBufferController->GetPacketLastPTS(BUFFER_TYPE_AUDIO);
            int64_t lastPos;

            if (lastVideoPos == INT64_MIN) {
                lastPos = lastAudioPos;
            } else if (lastAudioPos == INT64_MIN) {
                lastPos = lastVideoPos;
            } else {
                lastPos = lastAudioPos < lastVideoPos ? lastAudioPos : lastVideoPos;
            }

            lastPos -= min(mSet->RTMaxDelayTime, 500 * 1000);
            int64_t lastVideoKeyTimePos = mBufferController->GetKeyTimePositionBefore(BUFFER_TYPE_VIDEO, lastPos);
            int videoPacketCount = mBufferController->GetPacketSize(BUFFER_TYPE_VIDEO);

            if (videoPacketCount == 0 || lastVideoKeyTimePos != INT64_MIN) {
                AF_LOGD("drop left lastPts %lld, lastVideoKeyPts %lld", lastPos, lastVideoKeyTimePos);
                mMsgCtrlListener->ProcessSetSpeed(1.0);

                int64_t clearPos = (lastVideoKeyTimePos != INT64_MIN) ? lastVideoKeyTimePos : lastPos;
                int64_t dropVideoCount = mBufferController->ClearPacketBeforeTimePos(BUFFER_TYPE_VIDEO, clearPos);
                int64_t dropAudioCount = mBufferController->ClearPacketBeforeTimePos(BUFFER_TYPE_AUDIO, clearPos);

                if (dropVideoCount > 0) {
                    FlushVideoPath();
                    AF_LOGD("drop left video duration is %lld,left video size is %d",
                            mBufferController->GetPacketDuration(BUFFER_TYPE_VIDEO), mBufferController->GetPacketSize(BUFFER_TYPE_VIDEO));
                }

                if (dropAudioCount > 0) {
                    FlushAudioPath();
                    AF_LOGD("drop left aduio duration is %lld,left aduio size is %d",
                            mBufferController->GetPacketDuration(BUFFER_TYPE_AUDIO), mBufferController->GetPacketSize(BUFFER_TYPE_AUDIO));
                    mMasterClock.setTime(lastAudioPts);
                }
            }
        }

        int64_t lastAudio = mBufferController->GetPacketLastPTS(BUFFER_TYPE_AUDIO);

        if ((lastAudio != INT64_MIN) && (mPlayedAudioPts != INT64_MIN)) {
            int64_t delayTime = lastAudio - mPlayedAudioPts;
            static int64_t lastT = af_getsteady_ms();

            if (af_getsteady_ms() - lastT > 1000) {
                lastT = af_getsteady_ms();
                AF_LOGD("lastAudio:%lld mPlayedAudioPts:%lld, delayTime:%lld", lastAudio, mPlayedAudioPts, delayTime);
            }

            LiveCatchUp(delayTime);
        }

        break;
    }

    while (isTimeSync && mSuggestedPresentationDelay > 0) {
        if (!HAVE_AUDIO) {
            int64_t maxBufferDuration = getPlayerBufferDuration(true, false);

            if (maxBufferDuration > mSuggestedPresentationDelay + 1000 * 1000 * 5) {
                int64_t lastVideoPos = mBufferController->GetPacketLastTimePos(BUFFER_TYPE_VIDEO);
                lastVideoPos -= std::max(mSuggestedPresentationDelay, (int64_t)(500 * 1000ll));
                int64_t lastVideoKeyTimePos = mBufferController->GetKeyTimePositionBefore(BUFFER_TYPE_VIDEO, lastVideoPos);
                if (lastVideoKeyTimePos != INT64_MIN) {
                    mBufferController->ClearPacketBeforeTimePos(BUFFER_TYPE_VIDEO, lastVideoKeyTimePos);
                }
                break;
            }

            LiveTimeSync(maxBufferDuration);
            break;
        }

        int64_t maxBufferDuration = getPlayerBufferDuration(true, false);

        if (maxBufferDuration > mSuggestedPresentationDelay + 1000 * 1000 * 5) {
            //drop frame
            int64_t lastVideoPos = mBufferController->GetPacketLastTimePos(BUFFER_TYPE_VIDEO);
            int64_t lastAudioPos = mBufferController->GetPacketLastTimePos(BUFFER_TYPE_AUDIO);
            int64_t lastAudioPts = mBufferController->GetPacketLastPTS(BUFFER_TYPE_AUDIO);
            int64_t lastPos;

            if (lastVideoPos == INT64_MIN) {
                lastPos = lastAudioPos;
            } else if (lastAudioPos == INT64_MIN) {
                lastPos = lastVideoPos;
            } else {
                lastPos = lastAudioPos < lastVideoPos ? lastAudioPos : lastVideoPos;
            }

            lastPos -= std::max(mSuggestedPresentationDelay, (int64_t)(500 * 1000ll));
            int64_t lastVideoKeyTimePos = mBufferController->GetKeyTimePositionBefore(BUFFER_TYPE_VIDEO, lastPos);
            if (lastVideoKeyTimePos != INT64_MIN) {
                AF_LOGD("drop left lastPts %lld, lastVideoKeyPts %lld", lastPos, lastVideoKeyTimePos);
                mMsgCtrlListener->ProcessSetSpeed(1.0);
                int64_t dropVideoCount = mBufferController->ClearPacketBeforeTimePos(BUFFER_TYPE_VIDEO, lastVideoKeyTimePos);
                int64_t dropAudioCount = mBufferController->ClearPacketBeforeTimePos(BUFFER_TYPE_AUDIO, lastVideoKeyTimePos);

                if (dropVideoCount > 0) {
                    FlushVideoPath();
                    AF_LOGD("drop left video duration is %lld,left video size is %d",
                            mBufferController->GetPacketDuration(BUFFER_TYPE_VIDEO), mBufferController->GetPacketSize(BUFFER_TYPE_VIDEO));
                }

                if (dropAudioCount > 0) {
                    FlushAudioPath();
                    AF_LOGD("drop left aduio duration is %lld,left aduio size is %d",
                            mBufferController->GetPacketDuration(BUFFER_TYPE_AUDIO), mBufferController->GetPacketSize(BUFFER_TYPE_AUDIO));
                    mMasterClock.setTime(mBufferController->GetPacketPts(BUFFER_TYPE_AUDIO));
                }
            }
        }

        int64_t lastAudio = mBufferController->GetPacketLastPTS(BUFFER_TYPE_AUDIO);

        if ((lastAudio != INT64_MIN) && (mPlayedAudioPts != INT64_MIN)) {
            int64_t delayTime = lastAudio - mPlayedAudioPts;
            static int64_t lastT = af_getsteady_ms();

            if (af_getsteady_ms() - lastT > 1000) {
                lastT = af_getsteady_ms();
                AF_LOGD("lastAudio:%lld mPlayedAudioPts:%lld, delayTime:%lld", lastAudio, mPlayedAudioPts, delayTime);
            }

            LiveTimeSync(delayTime);
        }
        break;
    }

    //check buffering status
    if ((mBufferingFlag || mFirstBufferFlag)) {
        if (((cur_buffer_duration > HighBufferDur || (HighBufferDur >= mSet->maxBufferDuration && mBufferIsFull)) &&
             (!HAVE_VIDEO || videoDecoderFull || APP_BACKGROUND == mAppStatus)) ||
            mEof) {
            // if still in seek, wait for seek status be changed.
            if (!mSeekFlag || mEof) {
                if (mBufferingFlag) {
                    mLoadingProcess = -1;
                    mPNotifier->NotifyLoading(loading_event_end, 0);
                    AF_LOGD("loading end");

                    if (mPlayStatus == PLAYER_PLAYING) {
                        mMasterClock.start();
                        mAVDeviceManager->pauseAudioRender(false);
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

                if ((mTimeoutStartTime == INT64_MIN) || (PLAYER_PLAYING != mPlayStatus)) {
                    mTimeoutStartTime = curTime;
                } else if (curTime - mTimeoutStartTime >= mSet->timeout_ms) {
                    //TODO: demuxer still read crazy
                    if (mLowMem) {
                        mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_SYSTEM_LOW_MEMORY, "App Low memory");
                        // if it's a network error or waiting for network to retry, don't notify error
                    } else if (mSourceListener->isConnected() && !mSourceListener->isPending()) {
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
void SuperMediaPlayer::closeVideo()
{
    AF_LOGW("close video stream");
    mDemuxerService->CloseStream(mCurrentVideoIndex);
    mCurrentVideoIndex = -1;
    //  mVideoFrameQue.clear();
    mBufferController->ClearPacket(BUFFER_TYPE_VIDEO);
    FlushVideoPath();
}

void SuperMediaPlayer::LiveCatchUp(int64_t delayTime)
{
    int recoverGap = 50 * 1000;

    if (mSet->RTMaxDelayTime >= 1000 * 1000) {
        recoverGap = 500 * 1000;
    } else if (mSet->RTMaxDelayTime >= 200 * 1000) {
        recoverGap = 100 * 1000;
    }

    if ((delayTime > mSet->RTMaxDelayTime) && (150 * 1000 < delayTime)) {
        mMsgCtrlListener->ProcessSetSpeed(1.2);
    } else if ((delayTime < mSet->RTMaxDelayTime - recoverGap) || (100 * 1000 > delayTime)) {
        mMsgCtrlListener->ProcessSetSpeed(1.0);
    }
}

void SuperMediaPlayer::LiveTimeSync(int64_t delayTime)
{
    int64_t maxGopTime = mDemuxerService->getDemuxerHandle()->getMaxGopTimeUs();
    if (maxGopTime <= 0) {
        maxGopTime = 2 * 1000 * 1000;
    }
    if (maxGopTime > mSuggestedPresentationDelay) {
        maxGopTime = mSuggestedPresentationDelay;
    }
    int64_t bufferDelay = mSuggestedPresentationDelay;
    if ((delayTime > bufferDelay + maxGopTime * 0.5) && (delayTime > 150 * 1000)) {
        mMsgCtrlListener->ProcessSetSpeed(1.2);
        mLiveTimeSyncType = LiveTimeSyncType::LiveTimeSyncCatchUp;
        AF_LOGD("LiveTimeSync, delayTime=%lld, set speed 1.2", delayTime);
    } else if (delayTime < bufferDelay - maxGopTime * 1.3) {
        mMsgCtrlListener->ProcessSetSpeed(0.9);
        mLiveTimeSyncType = LiveTimeSyncType::LiveTimeSyncSlowDown;
        AF_LOGD("LiveTimeSync, delayTime=%lld, set speed 0.9", delayTime);
    }
    if (mLiveTimeSyncType == LiveTimeSyncType::LiveTimeSyncCatchUp) {
        if ((delayTime < bufferDelay - maxGopTime) || (delayTime < 100 * 1000)) {
            mMsgCtrlListener->ProcessSetSpeed(1.0);
            mLiveTimeSyncType = LiveTimeSyncType::LiveTimeSyncNormal;
            AF_LOGD("LiveTimeSync, delayTime=%lld, recover from catch up", delayTime);
        }
    } else if (mLiveTimeSyncType == LiveTimeSyncType::LiveTimeSyncSlowDown) {
        if (delayTime > bufferDelay) {
            mMsgCtrlListener->ProcessSetSpeed(1.0);
            mLiveTimeSyncType = LiveTimeSyncType::LiveTimeSyncNormal;
            AF_LOGD("LiveTimeSync, delayTime=%lld, recover from slow down", delayTime);
        }
    }
}

void SuperMediaPlayer::notifyPreparedCallback()
{
    if (waitingForStart && mSet->bLooping) {
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

    if (mSeekFlag && mPlayStatus != PLAYER_PREPARING) {
        if (HAVE_VIDEO && mAppStatus != APP_BACKGROUND) {
            rendered = RenderVideo(true);

            if (rendered) {
                AF_LOGD("TIMEPOS RenderVideo :%lld", mPlayedVideoPts / 1000);

                /*
                 *  set the position to video position tmp, audio will update the position when it rendered,
                 *  otherwise the position will update to the old audio position when audio not reach on time after seek.
                 *
                 */
                if (mSoughtVideoPos != INT64_MIN) {
                    mCurrentPos = mSoughtVideoPos;
                } else {
                    mCurrentPos = mPlayedVideoPts;
                }
                NotifyPosition(getCurrentPosition());

                // seek preview can't render audio,but set the audio clock to here pts
                if (HAVE_AUDIO && (mAudioTime.startTime <= 0)) {
                    if (!mAudioFrameQue.empty()) {
                        mAudioTime.startTime = mAudioFrameQue.front()->getInfo().pts;
                    } else {
                        mAudioTime.startTime = mPlayedVideoPts;
                    }
                }
            }
        } else {// audio only
            if (!mAudioFrameQue.empty()) {
                NotifyPosition(mAudioFrameQue.front()->getInfo().timePosition);
                rendered = true;
                mCurrentPos = mAudioFrameQue.front()->getInfo().timePosition;
                mAudioTime.startTime = mAudioFrameQue.front()->getInfo().pts;
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
        if (mEof && mAudioFrameQue.empty() && mBufferController->GetPacketSize(BUFFER_TYPE_AUDIO) == 0) {
            mMasterClock.setReferenceClock(nullptr, nullptr);
        }

        if (mSeekFlag) {
            mSeekFlag = false;

            if (!mMessageControl->findMsgByType(MSG_SEEKTO)) {
                // update position when seek end. in case of when paused.
                // update position before reset seek status, so getCurrentPosition return mSeekPos instead of mCurrentPos
                // fix bug the mCurrentPos not accuracy
                NotifyPosition(getCurrentPosition());
                ResetSeekStatus();
                mPNotifier->NotifySeekEnd(mSeekInCache);
                mSeekInCache = false;
            }
        }
    }
}

void SuperMediaPlayer::doDeCode()
{
    //get video packet to decode
    if (HAVE_VIDEO && !videoDecoderEOS && mAVDeviceManager->isDecoderValid(SMPAVDeviceManager::DEVICE_TYPE_VIDEO)) {
        int max_cache_size = VIDEO_PICTURE_MAX_CACHE_SIZE;

        if (mPictureCacheType == picture_cache_type_cannot) {
            max_cache_size = 1;
        }

        unsigned long videoFrameSize = mVideoFrameQue.size();

        if (videoFrameSize < max_cache_size) {
            int64_t startDecodeTime = af_getsteady_ms();
            int64_t videoEarlyUs = 0;

            do {
                if (mCanceled) {
                    break;
                }
                // if still in seeking, don't send data to decoder in background
                // due to the playback position could be changed later.
                if ((APP_BACKGROUND == mAppStatus) && isSeeking()) {
                    break;
                }

                if (mVideoPacket == nullptr) {
                    mVideoPacket = mBufferController->getPacket(BUFFER_TYPE_VIDEO);
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

                if (mVideoPacket && (!HAVE_AUDIO || audioDecoderEOS)) {
                    if (!(mAVDeviceManager->getDecoder(SMPAVDeviceManager::DEVICE_TYPE_VIDEO)->getFlags() & DECFLAG_PASSTHROUGH_INFO) &&
                        mVideoPacket->getInfo().timePosition >= 0) {
                        mCurrentPos = mVideoPacket->getInfo().timePosition;
                        //printTimePosition(mCurrentPos);
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
    if (HAVE_AUDIO && mAVDeviceManager->isDecoderValid(SMPAVDeviceManager::DEVICE_TYPE_AUDIO)) {

        while (mAudioFrameQue.size() < 2 && !audioDecoderEOS && !mCanceled) {

            if (mAudioPacket == nullptr) {
                mAudioPacket = mBufferController->getPacket(BUFFER_TYPE_AUDIO);
            }

            if (mAudioPacket) {
                int64_t timePosition = mAudioPacket->getInfo().timePosition;
                int ret = DecodeAudio(mAudioPacket);
                if (mAudioPacket == nullptr && timePosition >= 0 &&
                    !(mAVDeviceManager->getDecoder(SMPAVDeviceManager::DEVICE_TYPE_AUDIO)->getFlags() & DECFLAG_PASSTHROUGH_INFO)) {
                    mCurrentPos = timePosition;
                    //printTimePosition(mCurrentPos);
                }
                if (ret == -EAGAIN) {
                    break;
                }
            } else if (mEof) {
                unique_ptr<IAFPacket> packet{};
                DecodeAudio(packet);
            } else
                break;
        }

        //            AF_LOGD("mAudioFrameQue.size is %d\n", mAudioFrameQue.size());
    }
}

void SuperMediaPlayer::checkEOS()
{
    if (!mEof || PLAYER_COMPLETION == mPlayStatus) {
        return;
    }

    //in case of ONLY AUDIO stream.
    if ((HAVE_VIDEO && mAVDeviceManager->isDecoderValid(SMPAVDeviceManager::DEVICE_TYPE_VIDEO) && !videoDecoderEOS &&
         (APP_BACKGROUND != mAppStatus)) ||
        (HAVE_AUDIO && !audioDecoderEOS)) {
        return;
    }

    int packetSize = mBufferController->GetPacketSize(BUFFER_TYPE_AUDIO);
    int frameSize = static_cast<int>(mAudioFrameQue.size());

    if ((APP_BACKGROUND != mAppStatus) && mAVDeviceManager->isDecoderValid(SMPAVDeviceManager::DEVICE_TYPE_VIDEO)) {
        frameSize += mVideoFrameQue.size();
        packetSize += mBufferController->GetPacketSize(BUFFER_TYPE_VIDEO);
    }

    if (frameSize > 0 || packetSize > 0) {
        AF_TRACE;
        return;
    }

    uint64_t audioQueDuration = mAVDeviceManager->getAudioRenderQueDuration();

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

    NotifyPosition(mDuration);
    playCompleted();
}

void SuperMediaPlayer::playCompleted()
{
    //notify seek completion if seek to the end directly.
    if (mSeekFlag) {
        mSeekFlag = false;

        if (!mMessageControl->findMsgByType(MSG_SEEKTO)) {
            ResetSeekStatus();
            mPNotifier->NotifySeekEnd(mSeekInCache);
            mSeekInCache = false;
        }
    }

    if (mSet->bLooping && mDuration > 0) {
        mSeekPos = 0;//19644161: need reset seek position
        mMsgCtrlListener->ProcessSeekToMsg(0, false);
        mPNotifier->NotifyLoopStart();
        NotifyPosition(0);
    } else {
        if (mPlayStatus != PLAYER_COMPLETION) {
            if (APP_BACKGROUND == mAppStatus) {
                FlushVideoPath();
                mBufferController->ClearPacket(BUFFER_TYPE_VIDEO);
            }

            mPNotifier->NotifyCompletion();
            ChangePlayerStatus(PLAYER_COMPLETION);
            mUtil->reset();
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
            if ((pVideoPacket->getInfo().timePosition < checkPos) && (pVideoPacket->getInfo().timePosition < mDuration - 200 * 1000)) {
                pVideoPacket->setDiscard(true);
            }
        }

        if (!mRecorderSet->decodeFirstVideoFrameInfo.isFirstPacketSendToDecoder) {
            DecodeFirstFrameInfo &info = mRecorderSet->decodeFirstVideoFrameInfo;
            info.isFirstPacketSendToDecoder = true;
            info.firstPacketSize = pVideoPacket->getSize();
            info.firstPacketPts = pVideoPacket->getInfo().pts;
            info.waitFirstFrame = true;
            info.sendFirstPacketTimeMs = af_getsteady_ms();
        }

        ret = mAVDeviceManager->sendPacket(pVideoPacket, SMPAVDeviceManager::DEVICE_TYPE_VIDEO, 0);
        // don't need pop if need retry later
        if (!(ret & STATUS_RETRY_IN)) {
            //  mBufferController->PopFrontPacket(BUFFER_TYPE_VIDEO);
            assert(pVideoPacket == nullptr);
        }
    } else if (mEof) {
        //     mVideoDecoder->setEOF();
        mAVDeviceManager->sendPacket(pVideoPacket, SMPAVDeviceManager::DEVICE_TYPE_VIDEO, 0);
        ret = 0;
    }

    if (ret > 0) {
        bool haveError = false;

        if (ret & STATUS_HAVE_ERROR) {
            if (mAVDeviceManager->getDecoder(SMPAVDeviceManager::DEVICE_TYPE_VIDEO)->get_error_frame_no() > MAX_DECODE_ERROR_FRAME) {
                haveError = true;
            }
        }

        if (ret & STATUS_DRM_ERROR) {
            haveError = true;
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
    int ret = mAVDeviceManager->getFrame(pFrame, SMPAVDeviceManager::DEVICE_TYPE_VIDEO, 0);

    if (ret == STATUS_EOS) {
        videoDecoderEOS = true;

        if (mSeekFlag && mSeekNeedCatch) {
            mSeekNeedCatch = false;
        }
    }

    if (pFrame != nullptr) {

        if (mRecorderSet->decodeFirstVideoFrameInfo.waitFirstFrame) {
            DecodeFirstFrameInfo &info = mRecorderSet->decodeFirstVideoFrameInfo;
            info.getFirstFrameTimeMs = af_getsteady_ms();
            info.waitFirstFrame = false;
        }

        mAVDeviceManager->getDecoder(SMPAVDeviceManager::DEVICE_TYPE_VIDEO)->clean_error();

        if (mSecretPlayBack) {
            pFrame->setProtect(true);
        }
        int64_t pts = pFrame->getInfo().pts;

        if (mSeekFlag && mSeekNeedCatch) {
            mSeekNeedCatch = false;
        }

        auto *meta = (Stream_meta *) (mCurrentVideoMeta.get());

        if (!mAdaptiveVideo && mVideoWidth > 0 && (pFrame->getInfo().video.width != mVideoWidth || pFrame->getInfo().video.height != mVideoHeight)) {

            // displayWidth and displayHeight is changed, but we can't know the sar for now

            // TODO: get the sar from frame or update current video meta
            meta->displayWidth = meta->displayHeight = 0;
        }

        if (meta->displayWidth > 0 && meta->displayHeight > 0) {
            pFrame->getInfo().video.dar = 1.0 * meta->displayWidth / meta->displayHeight;
        } else {
            //if not get displaywidth/height , set dar with width/height
            pFrame->getInfo().video.dar = 1.0 * pFrame->getInfo().video.width / pFrame->getInfo().video.height;
        }

        mDemuxerService->SetOption("V_FRAME_DECODED", pts);
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
        int ret;
        do {
            ret = RenderAudio();
            if (RENDER_NONE != ret) {
                audioRendered = true;
            }
        } while (ret == RENDER_FULL);
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
    int64_t position = INT64_MIN;
    int render_ret;
//#define DUMP_PCM
#ifdef DUMP_PCM
    static int fd = open("out.pcm", O_CREAT | O_RDWR, 0666);
#endif

    if (mAudioFrameQue.empty()) {
        if (audioDecoderEOS && mAVDeviceManager->getAudioRenderQueDuration() == 0) {
            mMasterClock.setReferenceClock(nullptr, nullptr);
        }
        return ret;
    }

    pts = mAudioFrameQue.front()->getInfo().pts;
    position = mAudioFrameQue.front()->getInfo().timePosition;

    if (pts == INT64_MIN) {
        mAudioFrameQue.pop_front();
        return ret;
    }

    auto *avafFrame = dynamic_cast<AVAFFrame *>(mAudioFrameQue.front().get());

    if (avafFrame) {
        duration = getPCMFrameDuration(avafFrame->ToAVFrame());
    }

    if (mFrameCb && (!mSecretPlayBack || mDrmKeyValid)) {
        mFrameCb(mFrameCbUserData, avafFrame);
    }

    render_ret = mAVDeviceManager->renderAudioFrame(mAudioFrameQue.front(), 0);

    if (render_ret == IAudioRender::FORMAT_NOT_SUPPORT) {
        if (mAVDeviceManager->getAudioRenderQueDuration() == 0) {
            std::lock_guard<std::mutex> uMutex(mCreateMutex);
            mAudioTime.startTime = mAudioFrameQue.front()->getInfo().pts;
            mAudioTime.deltaTimeTmp = 0;
            mAudioTime.deltaTime = 0;
            mLastAudioFrameDuration = -1;
            setUpAudioRender(mAudioFrameQue.front()->getInfo().audio);
            if (mBRendingStart) {
                mAVDeviceManager->pauseAudioRender(false);
            }
            mAVDeviceManager->renderAudioFrame(mAudioFrameQue.front(), 0);
        }
    } else if (render_ret == IAudioRender::OPEN_AUDIO_DEVICE_FAILED) {
        AF_LOGE("render audio failed due to can not open device, close audio stream");
        mDemuxerService->CloseStream(mCurrentAudioIndex);
        mCurrentAudioIndex = -1;
        mMasterClock.setReferenceClock(nullptr, nullptr);
        mAudioFrameQue.clear();
        mBufferController->ClearPacket(BUFFER_TYPE_AUDIO);
        if (HAVE_VIDEO) {
            mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_OPEN_AUDIO_DEVICE_FAILED, "open audio device failed");
        } else {
            ChangePlayerStatus(PLAYER_ERROR);
            mPNotifier->NotifyError(MEDIA_PLAYER_ERROR_RENDER_AUDIO_OPEN_DEVICE_FAILED, "open audio device failed");
            return ret;
        }
    }

    if (!mAudioFrameQue.empty() && mAudioFrameQue.front() == nullptr) {
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
            if (!mAudioPtsRevert) {
                mAudioPtsRevert = pts < mPlayedAudioPts - mPtsDiscontinueDelta;
                if (mAudioPtsRevert) {
                    AF_LOGI("PTS_REVERTING audio start\n");
                }
            }
            int64_t offset = pts - (mPlayedAudioPts + mLastAudioFrameDuration);

            /*
             * the mLastAudioFrameDuration and the pts are all not the accurate value,
             * the mLastAudioFrameDuration accurate for 1/1000000 s,
             * the pts maybe accurate for 1/1000 s (eg. flv file), so can't increase the deltaTimeTmp when
             * offset little than 1ms.
             */
            if (llabs(offset)) {
                //    AF_LOGW("offset is %lld,pts is %lld", offset, pts);
                mAudioTime.deltaTimeTmp += offset;
                mPlayedAudioPts += offset;
            }

            if (llabs(mAudioTime.deltaTimeTmp) > 100000) {
                AF_LOGW("correct audio and master clock offset is %lld, frameDuration :%lld", mAudioTime.deltaTimeTmp,
                        mLastAudioFrameDuration);
                mAudioTime.deltaTime += mAudioTime.deltaTimeTmp;
                mAudioTime.deltaTimeTmp = 0;
            }
        }
    }

    if (mPlayedAudioPts == INT64_MIN && isSeeking()) {
        // update after send first frame in seeking, because audio render callback is async.
        // sometimes notify position before audio rendered callback , will cause position not right.
        mCurrentPos = position;
    }

    if (mPlayedAudioPts != INT64_MIN) {
        mPlayedAudioPts += duration;
    } else
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

    if (!mAVDeviceManager->isVideoRenderValid()) {
        return false;
    }
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

    int frameWidth;
    int frameHeight = videoFrame->getInfo().video.height;
    if (videoFrame->getInfo().video.dar != 0) {
        frameWidth = videoFrame->getInfo().video.dar * videoFrame->getInfo().video.height;
    } else {
        frameWidth = videoFrame->getInfo().video.width;
    }
    videoFrame->getInfo().video.rotate = mVideoRotation;

    if (!mVideoPtsRevert) {
        mVideoPtsRevert = mPlayedVideoPts != INT64_MIN && videoPts < mPlayedVideoPts - mPtsDiscontinueDelta;

        if (mVideoPtsRevert) {
            AF_LOGI("PTS_REVERTING video start\n");
        }
    }

    // audio pts first revert to small, force render the old video frame
    if (PTS_REVERTING && mAudioPtsRevert && videoPts - mPtsDiscontinueDelta > mPlayedAudioPts) {
        AF_LOGI("PTS_REVERTING force render the old video frame");
        force_render = true;
        // video pts first revert to small,the new video data wait audio pts to revert
    } else if (PTS_REVERTING && mVideoPtsRevert && videoPts + mPtsDiscontinueDelta < mPlayedAudioPts) {
        AF_LOGI("PTS_REVERTING wait audio to revert");
        return false;
    }

    int64_t masterPlayedTime = mMasterClock.GetTime();
    int64_t videoLateUs = masterPlayedTime - videoPts;// if videoLateUs > 0, video is late

    /*
         *  if stc is free, video rectify it
         */
    if ((llabs(videoLateUs) > 1000 * 1000) || (llabs(videoLateUs) > mSet->maxBufferDuration)) {
        // don't have master, or master not in valid status
        if (!mMasterClock.haveMaster() || !mMasterClock.isMasterValid()) {
            mMasterClock.setTime(videoPts);
            masterPlayedTime = videoPts;
        }
    }

    /* video early more than 10ms, don't (render||drop) it, deal it next time
         * if the pts is not continue drop it

         ---------------------------------------early----------------------------|------------------------late--------------

         ----------------------------------|-------------------------------|-----|-------------|--------------------------------------->
         drop if pts discontinue          10s       render next time     10ms   0            500ms     drop

                                                       |--------render-----|


         */
    bool render = force_render;

    if (!force_render) {
        if (videoLateUs < -10 * 1000 && (!mDemuxerService->getDemuxerHandle()->isTSDiscontinue() || videoLateUs > -mPtsDiscontinueDelta)) {
            return false;
        }

        // To avoid video frame packet overload when video decoder can't catch up.
        if (videoLateUs >= 500 * 1000 && !PTS_REVERTING) {
            int64_t lastVideoKeyPts = mBufferController->GetKeyPTSBefore(BUFFER_TYPE_VIDEO, masterPlayedTime);

            if (lastVideoKeyPts != INT64_MIN) {
                int64_t dropVideoCount = mBufferController->ClearPacketBeforePts(BUFFER_TYPE_VIDEO, lastVideoKeyPts);

                if (dropVideoCount > 0) {
                    FlushVideoPath();
                    AF_LOGD("videolaterUs is %lld,drop video count is %d", videoLateUs, dropVideoCount);
                    mVideoCatchingUp = true;
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
        } else if (videoLateUs < 500 * 1000) {// video early litter than 10ms or late litter than 500ms,render it
            render = true;
        }
        // Try to render if already haven't render more then 60ms
        else if (INT64_MIN == mPlayedVideoPts || (videoPts - mPlayedVideoPts) > 60 * 1000) {
            render = true;
        }
    }

    if (render) {
        mVideoCatchingUp = false;
        SendVideoFrameToRender(move(videoFrame));

        if (frameWidth != mVideoWidth || frameHeight != mVideoHeight) {
            mVideoWidth = frameWidth;
            mVideoHeight = frameHeight;
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
        videoFrame->setDiscard(true);
        mVideoCatchingUp = true;

        if (mFrameCb && (!mSecretPlayBack || mDrmKeyValid)) {
            mFrameCb(mFrameCbUserData, videoFrame.get());
        }
        RenderCallback(ST_TYPE_VIDEO, false, videoFrame->getInfo());
    }

    mPlayedVideoPts = videoPts;
    mVideoFrameQue.pop();
    return render;
}

void SuperMediaPlayer::RenderSubtitle(int64_t pts)
{
    if (mSubPlayer && mSubPlayer->isActive()) {
        mSubPlayer->update(getCurrentPosition());
        return;
    }

    auto iter = mSubtitleShowedQueue.begin();

    while (iter != mSubtitleShowedQueue.end()) {
        if (*iter) {
            if (((*iter)->getInfo().pts + (*iter)->getInfo().duration) <= pts) {
                mPNotifier->NotifySubtitleEvent(subTitle_event_hide, (*iter).release(), 0, nullptr);
                iter = mSubtitleShowedQueue.erase(iter);
                continue;
            }
        }

        iter++;
    }

    int64_t subTitlePts = mBufferController->GetPacketPts(BUFFER_TYPE_SUBTITLE);

    if (subTitlePts == INT64_MIN || subTitlePts > pts) {
        return;
    }

    unique_ptr<IAFPacket> pFrame = mBufferController->getPacket(BUFFER_TYPE_SUBTITLE);

    if (pFrame == nullptr) {
        return;
    }

    bool changed = false;

    if (pFrame->getInfo().pts + pFrame->getInfo().duration < pts) {
        AF_LOGD("read subtitle pts is long before pts is %lld,subtitle pts is %lld", pts, subTitlePts);
        mBufferController->ClearPacketBeforePts(BUFFER_TYPE_SUBTITLE, subTitlePts);

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
        /*
             * if have seek not completed,DO NOT update the position,it will lead process bar
             * jumping
             */
        if ((mPlayStatus == PLAYER_PLAYING) && !isSeeking()) {
            //AF_LOGD("TIMEPOS OnTimer :%lld", getCurrentPosition());
            NotifyPosition(getCurrentPosition());
        }

        PostBufferPositionMsg();
    }
    mPNotifier->NotifyCurrentDownloadSpeed(mUtil->getCurrentDownloadSpeed());
}

void SuperMediaPlayer::SendVideoFrameToRender(unique_ptr<IAFFrame> frame, bool valid)
{
    if (mFrameCb && (!mSecretPlayBack || mDrmKeyValid)) {
        bool rendered = mFrameCb(mFrameCbUserData, frame.get());
        if (rendered) {
            RenderCallback(ST_TYPE_VIDEO, true, frame->getInfo());
            return;
        }
    }
    if (mAVDeviceManager->isVideoRenderValid()) {
        int ret = mAVDeviceManager->renderVideoFrame(frame);

        if (ret < 0) {
            AF_LOGE("renderFrame error \n");
            // for windows init failed, which may need change render type in future.
            mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_VIDEO_RENDER_INIT_ERROR, "init video render failed");
        }
    } else {
        assert(0);
        //render directly
        RenderCallback(ST_TYPE_VIDEO, !frame->getDiscard(), frame->getInfo());
    }
}

int SuperMediaPlayer::DecodeAudio(unique_ptr<IAFPacket> &pPacket)
{
    if (audioDecoderEOS) {
        return 0;
    }

    unique_ptr<IAFFrame> frame{};
    int ret;

    do {
        ret = mAVDeviceManager->getFrame(frame, SMPAVDeviceManager::DEVICE_TYPE_AUDIO, 0);
        if (ret == STATUS_EOS) {
            audioDecoderEOS = true;
            break;
        }

        if (frame != nullptr) {

            int64_t duration = mBufferController->GetPacketDuration(BUFFER_TYPE_AUDIO);
            if (duration < 0) {
                //If audio duration is unknow when demux , update duration after decode one frame.
                IAFFrame::AFFrameInfo frameInfo = frame->getInfo();
                int64_t packetDuration = (int64_t) frameInfo.audio.nb_samples * 1000000 / frameInfo.audio.sample_rate;
                mBufferController->SetOnePacketDuration(BUFFER_TYPE_AUDIO, packetDuration);
            }

            if (mRecorderSet->decodeFirstAudioFrameInfo.waitFirstFrame) {
                DecodeFirstFrameInfo &info = mRecorderSet->decodeFirstAudioFrameInfo;
                info.getFirstFrameTimeMs = af_getsteady_ms();
                info.waitFirstFrame = false;
            }

            if (mSecretPlayBack) {
                frame->setProtect(true);
            }
            if (frame->getInfo().pts == INT64_MIN) {
                // TODO: why mAudioFrameQue.back()->getInfo().pts is INT64_MIN
                if (!mAudioFrameQue.empty() && mAudioFrameQue.back()->getInfo().pts != INT64_MIN) {
                    double duration = ((double) frame->getInfo().audio.nb_samples) / frame->getInfo().audio.sample_rate;
                    frame->getInfo().pts = mAudioFrameQue.back()->getInfo().pts + duration * 1000000;
                } else {
                    //                       assert(0);
                }
            }
            mDemuxerService->SetOption("A_FRAME_DECODED", frame->getInfo().pts);
            mAudioFrameQue.push_back(move(frame));
        }
    } while (ret != -EAGAIN && ret != -EINVAL);

    if (!mRecorderSet->decodeFirstAudioFrameInfo.isFirstPacketSendToDecoder) {
        DecodeFirstFrameInfo &info = mRecorderSet->decodeFirstAudioFrameInfo;
        info.isFirstPacketSendToDecoder = true;
        info.waitFirstFrame = true;
        info.firstPacketSize = pPacket->getSize();
        info.firstPacketPts = pPacket->getInfo().pts;
        info.sendFirstPacketTimeMs = af_getsteady_ms();
    }

    ret = mAVDeviceManager->sendPacket(pPacket, SMPAVDeviceManager::DEVICE_TYPE_AUDIO, 0);

    if (ret > 0) {
        bool haveError = false;

        if (ret & STATUS_HAVE_ERROR) {
            if (mAVDeviceManager->getDecoder(SMPAVDeviceManager::DEVICE_TYPE_AUDIO)->get_error_frame_no() > MAX_DECODE_ERROR_FRAME) {
                haveError = true;
            }
        }

        if (ret & STATUS_DRM_ERROR) {
            haveError = true;
        }

        if (ret & STATUS_CREATE_FAIL) {
            haveError = true;
        }

        if (ret &= STATUS_RETRY_IN) {
            ret = -EAGAIN;
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
    std::deque<StreamInfo *> &streamInfoQueue = mMediaInfo.mStreamInfoQueue;
    int streamCount = (int) streamInfoQueue.size();
    int videoStreams = 0;

    for (int i = 0; i < streamCount; i++) {
        StreamInfo *info = streamInfoQueue[i];

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

                if (!mSet->bDisableVideo && meta->type == STREAM_TYPE_VIDEO && mCurrentVideoIndex < 0 && meta->height > 0 &&
                    meta->attached_pic == 0) {
                    AF_LOGD("get a video stream\n");
                    mCurrentVideoIndex = GEN_STREAM_ID(mMainStreamId, j);
                    mVideoInterlaced = meta->interlaced;
                    updateVideoMeta();
                } else if (!mSet->bDisableAudio && meta->type == STREAM_TYPE_AUDIO && mCurrentAudioIndex < 0 && meta->channels > 0) {
                    AF_LOGD("get a audio stream\n");
                    mCurrentAudioIndex = GEN_STREAM_ID(mMainStreamId, j);
                    mCATimeBase = meta->ptsTimeBase;
                } else if (meta->type == STREAM_TYPE_SUB && mCurrentSubtitleIndex < 0) {
                    AF_LOGD("get a subtitle stream\n");
                    mCurrentSubtitleIndex = GEN_STREAM_ID(mMainStreamId, j);
                }
            }
        }
    }

    mPNotifier->CancelNotifyMediaInfo();
    //post before PostMsg(PreparedReq). sdk will get these infos on Prepared callback.
    mPNotifier->NotifyMediaInfo(&mMediaInfo);
}

void SuperMediaPlayer::setUpAVPath()
{
    if (!mInited) {
        return;
    }

    if (!mSet->mFastStart && mPlayStatus < PLAYER_PLAYING) {
        AF_LOGI("not fast start mode\n");
        return;
    }

    if (mCurrentAudioIndex >= 0 &&
        (!mAVDeviceManager->isDecoderValid(SMPAVDeviceManager::DEVICE_TYPE_AUDIO) || !mAVDeviceManager->isAudioRenderValid())) {
        AF_LOGD("SetUpAudioPath start");
        int ret = SetUpAudioPath();

        if (ret < 0) {
            AF_LOGE("%s SetUpAudioPath failed,url is %s %s", __FUNCTION__, mSet->url.c_str(), framework_err2_string(ret));
            mDemuxerService->CloseStream(mCurrentAudioIndex);
            mCurrentAudioIndex = -1;
            mCATimeBase = 0;
        } else {
        }
    }

    if (mCurrentVideoIndex >= 0) {
        int ret = SetUpVideoPath();

        if (ret < 0) {
            AF_LOGE("%s SetUpVideoPath failed,url is %s %s", __FUNCTION__, mSet->url.c_str(), framework_err2_string(ret));
            closeVideo();
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
        if (mBufferController->GetPacketDuration(BUFFER_TYPE_SUBTITLE) <= 0) {
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

    if (mPtsDiscontinueDelta == INT64_MIN) {
        int64_t maxGopTimeUs = mDemuxerService->getDemuxerHandle()->getMaxGopTimeUs();
        if (maxGopTimeUs > 0) {
            mPtsDiscontinueDelta = maxGopTimeUs;
        } else {
            mPtsDiscontinueDelta = PTS_DISCONTINUE_DELTA;
        }
        AF_LOGI("mPtsDiscontinueDelta = %lld", mPtsDiscontinueDelta);
    }

    // FIXME: transfer to frame
    if (pMedia_Frame->isProtected() && !mSecretPlayBack) {
        AF_LOGI("SecretPlayBack\n");
        mSecretPlayBack = true;

        if (!pMedia_Frame->getMagicKey().empty() && pMedia_Frame->getMagicKey() == mSet->drmMagicKey) {
            mDrmKeyValid = true;
        }
    }

    pFrame = pMedia_Frame.get();
    mUtil->notifyRead(MediaPlayerUtil::readEvent_Got, pFrame->getSize());

    // TODO: get the min first stream pts
    if (pFrame->getInfo().timePosition >= 0 && mMediaStartPts == INT64_MIN && pFrame->getInfo().streamIndex != mCurrentSubtitleIndex &&
        pFrame->getInfo().streamIndex != mWillChangedSubtitleStreamIndex) {
        mMediaStartPts = pFrame->getInfo().pts - pFrame->getInfo().timePosition;
    }

    if (mSeekFlag && mSeekNeedCatch) {
        if (pFrame->getInfo().timePosition < (mSeekPos - mSet->maxASeekDelta)) {
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

    std::deque<StreamInfo *> &streamInfoQueue = mMediaInfo.mStreamInfoQueue;
    if (id < streamInfoQueue.size() && streamInfoQueue[id]->type == ST_TYPE_VIDEO && mMainStreamId != -1 && id != mMainStreamId) {
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
                mWATimeBase = meta->ptsTimeBase;
            } else if (meta->type == STREAM_TYPE_SUB && streamId != mCurrentSubtitleIndex) {
                mWillChangedSubtitleStreamIndex = streamId;
            }
        }

        mMainStreamId = id;
    }

    if (!mInited) {
        ProcessOpenStreamInit(pFrame->getInfo().streamIndex);
        ProcessUpdateView();
        mInited = true;
    }

    //        AF_LOGD("read packet pts is %lld,streamIndex is %d duration is %d\n", pFrame->getInfo().pts, pFrame->getInfo().streamIndex,
    //                pFrame->getInfo().duration);

    if (pFrame->getInfo().streamIndex == mCurrentVideoIndex || pFrame->getInfo().streamIndex == mWillChangedVideoStreamIndex) {

        if (mMediaFrameCb && (!pMedia_Frame->isProtected() || mDrmKeyValid)) {
            mMediaFrameCb(mMediaFrameCbArg, pMedia_Frame.get(), ST_TYPE_VIDEO);
        }

        mBufferController->AddPacket(move(pMedia_Frame), BUFFER_TYPE_VIDEO);
        mDemuxerService->SetOption("V_FRAME_RECEIVE", pFrame->getInfo().pts);

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
            if (mSeekNeedCatch) {
                mSoughtVideoPos = mSeekPos;
            } else {
                mSoughtVideoPos = pFrame->getInfo().timePosition;
            }

            /*
                 * seek would clean the packet which have ExtraData in decoder queue,
                 * so add the ExtraData after seek on key frame
                 */
            if (/*mAdaptiveVideo &&*/ pFrame->getInfo().flags) {
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
            mCATimeBase = mWATimeBase;
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

        if (mMediaFrameCb && (!pMedia_Frame->isProtected() || mDrmKeyValid)) {
            mMediaFrameCb(mMediaFrameCbArg, pMedia_Frame.get(), ST_TYPE_AUDIO);
        }

        mBufferController->AddPacket(move(pMedia_Frame), BUFFER_TYPE_AUDIO);
        mDemuxerService->SetOption("A_FRAME_RECEIVE", pFrame->getInfo().pts);
    } else if (pFrame->getInfo().streamIndex == mCurrentSubtitleIndex || pFrame->getInfo().streamIndex == mWillChangedSubtitleStreamIndex) {
        if (mMediaFrameCb && (!pMedia_Frame->isProtected() || mDrmKeyValid)) {
            mMediaFrameCb(mMediaFrameCbArg, pMedia_Frame.get(), ST_TYPE_SUB);
        }

        mBufferController->AddPacket(move(pMedia_Frame), BUFFER_TYPE_SUBTITLE);
        AF_LOGD("read subtitle pts is %lld", pFrame->getInfo().pts);

        if (pFrame->getInfo().streamIndex == mWillChangedSubtitleStreamIndex) {
            mCurrentSubtitleIndex = mWillChangedSubtitleStreamIndex;
            mWillChangedSubtitleStreamIndex = -1;
        }
        if (mSubtitleChangedFirstPts == INT64_MAX) {
            mSubtitleChangedFirstPts = pFrame->getInfo().pts;
        }
    } else {
        AF_LOGD("unknown stream %x, read packet pts is %lld\n", pFrame->getInfo().streamIndex, pFrame->getInfo().pts);
        if (mCurrentAudioIndex < 0 || mCurrentVideoIndex < 0) {
            std::unique_ptr<streamMeta> meta;
            int ret1 = mDemuxerService->GetStreamMeta(meta, pFrame->getInfo().streamIndex, true);
            if (ret1 >= 0) {
                switch (((Stream_meta *) (*meta))->type) {
                    case STREAM_TYPE_VIDEO: {
                        if (!mSet->bDisableVideo && mCurrentVideoIndex < 0 && ((Stream_meta *) (*meta))->width > 0) {
                            mCurrentVideoIndex = pFrame->getInfo().streamIndex;
                            updateVideoMeta();
                        }
                        break;
                    }
                    case STREAM_TYPE_AUDIO: {
                        if (!mSet->bDisableAudio && mCurrentAudioIndex < 0 && ((Stream_meta *) (*meta))->channels > 0) {
                            mCurrentAudioIndex = pFrame->getInfo().streamIndex;
                            mCATimeBase = ((Stream_meta *) (*meta))->ptsTimeBase;
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }

    if (mWillSwitchVideo) {
        int videoCount = 0;
        int64_t startTime = mBufferController->FindSeamlessPointTimePosition(BUFFER_TYPE_VIDEO, videoCount);

        if (startTime == 0 || videoCount < 40) {
            return ret;
        }

        if (mMixMode) {
            int64_t startTimeA = mBufferController->FindSeamlessPointTimePosition(BUFFER_TYPE_AUDIO, videoCount);

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
    mAVDeviceManager->flushDevice(SMPAVDeviceManager::DEVICE_TYPE_AUDIO);

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
    mAVDeviceManager->flushDevice(SMPAVDeviceManager::DEVICE_TYPE_VIDEO);

    videoDecoderEOS = false;
    //flush frame queue

    while (!mVideoFrameQue.empty()) {
        mVideoFrameQue.front()->setDiscard(true);
        mMsgCtrlListener->ProcessRenderedMsg(ST_TYPE_VIDEO, mVideoFrameQue.front()->getInfo(), af_getsteady_ms(), false, nullptr);
        mVideoFrameQue.pop();
    }

    mPlayedVideoPts = INT64_MIN;
    mCurVideoPts = INT64_MIN;
    videoDecoderFull = false;
    mVideoPtsRevert = false;
    mVideoPacket = nullptr;
    dropLateVideoFrames = false;
    mVideoCatchingUp = false;
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
    if (mSubPlayer) {
        mSubPlayer->flush();
    }
}

void SuperMediaPlayer::PostBufferPositionMsg()
{
    if (mPlayStatus == PLAYER_PAUSED || mPlayStatus == PLAYER_PLAYING) {
        int64_t duration;

        if (isSeeking()) {
            duration = 0;
        } else {
            duration = getPlayerBufferDuration(false, false);
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

int64_t SuperMediaPlayer::getPlayerBufferDuration(bool gotMax, bool internal)
{
    int64_t durations[3] = {-1, -1, -1};
    int i = 0;
    int64_t duration = -1;

    if (HAVE_VIDEO) {
        int64_t &duration_c = durations[i++];
        duration_c = mBufferController->GetPacketDuration(BUFFER_TYPE_VIDEO);

        //            AF_LOGD("videoDuration is %lld\n",videoDuration);
        if (duration_c < 0 && !HAVE_AUDIO) {
            duration_c = mBufferController->GetPacketLastPTS(BUFFER_TYPE_VIDEO) - mBufferController->GetPacketPts(BUFFER_TYPE_VIDEO);

            if (duration_c <= 0) {
                duration_c = (int64_t) mBufferController->GetPacketSize(BUFFER_TYPE_VIDEO) * 40 * 1000;
            }
        }
        if (!internal && mDemuxerService && mDemuxerService->getDemuxerHandle()) {
            duration_c += mDemuxerService->getDemuxerHandle()->getBufferDuration(mCurrentVideoIndex);
        }

        if (mAVDeviceManager->isDecoderValid(SMPAVDeviceManager::DEVICE_TYPE_VIDEO)) {
            // FIXME: get the accurate duration
            duration_c += mAVDeviceManager->getDecoder(SMPAVDeviceManager::DEVICE_TYPE_VIDEO)->getInputPaddingSize() * 40 * 1000;
        }
    }

    if (HAVE_AUDIO) {
        int64_t &duration_c = durations[i++];
        duration_c = mBufferController->GetPacketDuration(BUFFER_TYPE_AUDIO);
        //            AF_LOGD("audioDuration is %lld\n",audioDuration);
        if (!internal && mDemuxerService && mDemuxerService->getDemuxerHandle()) {
            duration_c += mDemuxerService->getDemuxerHandle()->getBufferDuration(mCurrentAudioIndex);
        }

        if (mAVDeviceManager->isDecoderValid(SMPAVDeviceManager::DEVICE_TYPE_AUDIO)) {
            int64_t audioPacketDuration = mBufferController->GetOnePacketDuration(BUFFER_TYPE_AUDIO);
            if (audioPacketDuration <= 0) {
                audioPacketDuration = 23 * 1000;
            }
            duration_c += mAVDeviceManager->getDecoder(SMPAVDeviceManager::DEVICE_TYPE_AUDIO)->getInputPaddingSize() * audioPacketDuration;
        }
    }

    /*
         *  Do not let player loading when switching subtitle, we'll read subtitle first
         *  in ReadPacket()
         */
    if (HAVE_SUBTITLE && !mSubtitleEOS && mSubtitleChangedFirstPts == INT64_MIN) {
        int64_t &duration_c = durations[i++];
        duration_c = mBufferController->GetPacketDuration(BUFFER_TYPE_SUBTITLE);
        if (!internal && mDemuxerService && mDemuxerService->getDemuxerHandle()) {
            duration_c += mDemuxerService->getDemuxerHandle()->getBufferDuration(mCurrentSubtitleIndex);
        }
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
    int64_t audioLastPos = mBufferController->GetPacketLastTimePos(BUFFER_TYPE_AUDIO);
    int64_t videoLastPos = mBufferController->GetPacketLastTimePos(BUFFER_TYPE_VIDEO);
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

    //seek bigger than last frame
    if (minLastPos > 0 && pos > minLastPos) {
        return false;
    }

    //seek back
    if (pos <= mCurrentPos) {
        mBufferController->Rewind(BUFFER_TYPE_ALL);
        int64_t audioFirstPos = mBufferController->GetPacketFirstTimePos(BUFFER_TYPE_AUDIO);
        int64_t videoFirstPos = mBufferController->GetPacketFirstTimePos(BUFFER_TYPE_VIDEO);
        int64_t maxFirstPos = -1;
        if (HAVE_VIDEO && HAVE_AUDIO) {
            maxFirstPos = std::max(audioFirstPos, videoFirstPos);
        } else if (HAVE_VIDEO) {
            maxFirstPos = videoFirstPos;
        } else if (HAVE_AUDIO) {
            maxFirstPos = audioFirstPos;
        } else {
            // no video and audio ?
        }

        if (maxFirstPos > 0 && pos < maxFirstPos) {
            return false;
        }
    }

    int64_t keyPosBefore = INT64_MIN;

    //can not find last key video pts,return
    if (HAVE_VIDEO) {
        keyPosBefore = mBufferController->GetKeyTimePositionBefore(BUFFER_TYPE_VIDEO, pos);
    } else if (HAVE_AUDIO) {
        keyPosBefore = mBufferController->GetKeyTimePositionBefore(BUFFER_TYPE_AUDIO, pos);
    }

    if (keyPosBefore == INT64_MIN) {
        return false;
    }

    mBufferController->ClearPacketBeforeTimePos(BUFFER_TYPE_ALL, keyPosBefore);
    mSoughtVideoPos = keyPosBefore;
    return true;
}

void SuperMediaPlayer::SwitchVideo(int64_t startTime)
{
    AF_LOGD("video change find start time is %lld", startTime);
    int ret = mDemuxerService->OpenStream(mWillChangedVideoStreamIndex);

    if (ret < 0) {
        AF_LOGD("video", "switch video open stream failed,stream index %d\n", mCurrentVideoIndex);
        return;
    }

    if (mMixMode) {
        mDemuxerService->CloseStream(GEN_STREAM_INDEX(mCurrentVideoIndex));
    } else {
        mDemuxerService->CloseStream(mCurrentVideoIndex);
    }

    mDemuxerService->Seek(startTime / 1000 * 1000, 0, mWillChangedVideoStreamIndex);

    if (mMixMode) {
        mBufferController->ClearPacketAfterTimePosition(BUFFER_TYPE_AV, startTime);
    } else {
        mBufferController->ClearPacketAfterTimePosition(BUFFER_TYPE_VIDEO, startTime);
    }

    mWillSwitchVideo = false;
    mVideoChangedFirstPts = INT64_MAX;
    mEof = false;
}

int64_t SuperMediaPlayer::getAudioPlayTimeStampCB(void *arg)
{
    auto *pHandle = static_cast<SuperMediaPlayer *>(arg);
    return pHandle->getAudioPlayTimeStamp();
}

int64_t SuperMediaPlayer::getAudioPlayTimeStamp()
{
    if (!mAVDeviceManager->isAudioRenderValid()) {
        return INT64_MIN;
    }

    if (mSeekFlag) {
        return INT64_MIN;
    }

    int64_t aoutPos;
    aoutPos = mAVDeviceManager->getAudioRenderPosition();
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
    int ret = 0;

    if (meta->samplerate <= 0) {// meta.frame_size maybe 0 when playing artp
        ret = 0;
        return 0;
    }

    if (meta->duration > mDuration) {
        mDuration = meta->duration;
    }

    //setVolume to current setting after create new.
    SetVolume(mSet->mVolume);

    if (mSet->bMute) {
        mMsgCtrlListener->ProcessMuteMsg();
    }

    uint64_t flags = DECFLAG_SW;

#ifdef ANDROID
    bool isWideVineVideo = (meta->keyFormat != nullptr && strcmp(meta->keyFormat, "urn:uuid:edef8ba9-79d6-4ace-a3c8-27dcd51d21ed") == 0);
    if (isWideVineVideo) {
        flags |= DECFLAG_HW;
    }
#endif

    ret = mAVDeviceManager->setUpDecoder(flags, meta, nullptr, SMPAVDeviceManager::DEVICE_TYPE_AUDIO, 0);

    if (ret < 0) {
        MediaPlayerEventType type = MEDIA_PLAYER_EVENT_AUDIO_DECODER_DEVICE_ERROR;
        if (ret == gen_framework_errno(error_class_codec, codec_error_audio_not_support)) {
            type = MEDIA_PLAYER_EVENT_AUDIO_CODEC_NOT_SUPPORT;
        }
        AF_LOGE("setUpAudioDecoder error %d\n", ret);
        mPNotifier->NotifyEvent(type, framework_err2_string(ret));
        return ret;
    }

    return ret;
}

int SuperMediaPlayer::SetUpAudioPath()
{
    int ret = 0;
    if (!mAVDeviceManager->isDecoderValid(SMPAVDeviceManager::DEVICE_TYPE_AUDIO)) {

        /*
         * make sure the audio stream is opened before get stream meta,
         * otherwise, will crash in hls stream
         */
        if (mBufferController->IsPacketEmtpy(BUFFER_TYPE_AUDIO)) {
            return 0;
        }

        unique_ptr<streamMeta> pMeta{};
        mDemuxerService->GetStreamMeta(pMeta, mCurrentAudioIndex, false);
        auto *meta = (Stream_meta *) (pMeta.get());

        int64_t startTimeMs = af_getsteady_ms();

        ret = setUpAudioDecoder(meta);

        int64_t costTimeMs = af_getsteady_ms() - startTimeMs;
        mRecorderSet->createAudioDecoderCostMs = costTimeMs;

        if (ret < 0) {
            return ret;
        }
    }

    if (mAudioFrameQue.empty() || mAVDeviceManager->isAudioRenderValid()) {
        return 0;
    }

    //        IAFFrame::audioInfo info = ;
    //        info.channels = meta->channels;
    //        info.sample_rate = meta->samplerate;
    //        info.format = meta->sample_fmt;
    //        info.nb_samples = meta->frame_size;
    //        info.channel_layout = meta->channel_layout;
    setUpAudioRender(mAudioFrameQue.front()->getInfo().audio);
    return ret;
}

int SuperMediaPlayer::setUpAudioRender(const IAFFrame::audioInfo &info)
{
    int ret = mAVDeviceManager->setUpAudioRender(info);

    if (ret < 0) {
        AF_LOGE("AudioOutHandle Init Error is %d", ret);
        // don't release audio handle because we only new it in constructor
        // PS: we should try to recover it later, or notify error
        //                    mAudioOutHandle = 0;
        mCurrentAudioIndex = -1;
        mCATimeBase = 0;
        return -1;
    }
    mAVDeviceManager->setAudioRenderListener(mAudioRenderCB.get());
    mAVDeviceManager->setSpeed(mSet->rate);
    mAVDeviceManager->setMute(mSet->bMute);
    mAVDeviceManager->setVolume(mSet->mVolume);

    if (!mSecretPlayBack) {
        mAVDeviceManager->setAudioRenderingCb(mAudioRenderingCb, mAudioRenderingCbUserData);
        mAVDeviceManager->setVideoRenderingCb(mVideoRenderingCb, mVideoRenderingCbUserData);
    }
    return 0;
}

int SuperMediaPlayer::setUpVideoRender(uint64_t flags)
{
    if (mAppStatus == APP_BACKGROUND) {
        AF_LOGW("create video render in background");
    }
    AF_LOGD("SetUpVideoRender start");
    CreateVideoRender(flags);
    if (!mAVDeviceManager->isVideoRenderValid()) {
        AF_LOGE("can't create video render\n");
        mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_VIDEO_RENDER_INIT_ERROR, "init video render failed");
        return -EINVAL;
    }

    //re set view in case for not set view before
    if (mSet->mView) {
        if (mAVDeviceManager->isVideoRenderValid()) {
            mAVDeviceManager->getVideoRender()->setDisPlay(mSet->mView);
        }
    }

    return 0;
}


int SuperMediaPlayer::SetUpVideoPath()
{
    if (mAVDeviceManager->isDecoderValid(SMPAVDeviceManager::DEVICE_TYPE_VIDEO) && (mAVDeviceManager->isVideoRenderValid())) {
        return 0;
    }

    if (mBufferController->IsPacketEmtpy(BUFFER_TYPE_VIDEO)) {
        return 0;
    }

    if (mVideoInterlaced == InterlacedType_UNKNOWN) {
        AF_LOGW("Wait for parser video interlaced Type");
        return 0;
    }
    /*
     * update the video meta after the first video packet was reached,
     * otherwise the video meta is incomplete when playing a master hls playList.
     */
    updateVideoMeta();
    auto *meta = (Stream_meta *) (mCurrentVideoMeta.get());

    bool bHW = false;
    if (mSet->bEnableHwVideoDecode) {
        switch (meta->codec) {
            case AF_CODEC_ID_H264: {
                string value = getProperty("ro.video.dec.h264");
                bHW = !(value == "OFF");
                break;
            }
            case AF_CODEC_ID_HEVC: {
                string value = getProperty("ro.video.dec.hevc");
                bHW = !(value == "OFF");
                break;
            }

            default:
                bHW = true;
                break;
        }
    }

    bool tunnelRender = mSet->bEnableTunnelRender;
    if (!mSet->bEnableHwVideoDecode || !bHW) {
        //soft decoder not support tunnel Render
        tunnelRender = false;
    }

    uint64_t flags = 0;


    if (isHDRVideo(meta)) {
        /*
         * HDR video must use mediaCodec to render direct on Android,
         * we use a dummy render to release the frame simply
         */
#ifdef ANDROID
        flags |= IVideoRender::FLAG_DUMMY;
#else
        flags |= IVideoRender::FLAG_HDR;
#endif
    }
#ifdef ANDROID
    bool isWideVine = isWideVineVideo(meta);
#endif

    if (tunnelRender
#ifdef ANDROID
        || isWideVine
#endif
    ) {
        flags |= IVideoRender::FLAG_DUMMY;
    }

    int ret = setUpVideoRender(flags);
    if (ret < 0) {
        return ret;
    }

    if (mAVDeviceManager->isDecoderValid(SMPAVDeviceManager::DEVICE_TYPE_VIDEO)) {
        return 0;
    }

    AF_LOGD("SetUpVideoDecoder start");

    if (meta->interlaced == InterlacedType_UNKNOWN) {
        meta->interlaced = mVideoInterlaced;
    }

#ifdef ANDROID
    if (bHW && mAVDeviceManager->isVideoRenderValid() && mAVDeviceManager->getVideoRender()->getFlags() & IVideoRender::FLAG_DUMMY &&
        mSet->mView == nullptr) {
        return 0;
    }
#endif

    int64_t startTimeMs = af_getsteady_ms();
    ret = CreateVideoDecoder(bHW, *meta);

    if (ret < 0) {
        if (bHW) {

            if (flags & IVideoRender::FLAG_DUMMY) {
                flags &= ~IVideoRender::FLAG_DUMMY;
                ret = setUpVideoRender(flags);
                if (ret < 0) {
                    return ret;
                }
            }

            ret = CreateVideoDecoder(false, *meta);
        }
    }

    int64_t costTimeMs = af_getsteady_ms() - startTimeMs;
    mRecorderSet->createVideoDecoderCostMs = costTimeMs;

    if (ret < 0) {
        AF_LOGE("%s CreateVideoDecoder failed, error msg is  %s", __FUNCTION__, framework_err2_string(ret));

        if (ret == gen_framework_errno(error_class_codec, codec_error_video_not_support)) {
            mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_VIDEO_CODEC_NOT_SUPPORT, framework_err2_string(ret));
        } else if (ret == gen_framework_errno(error_class_codec, codec_error_video_device_error)) {
            mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_VIDEO_DECODER_DEVICE_ERROR, framework_err2_string(ret));
        }

        return ret;
    }

    if (mAVDeviceManager->getVideoDecoderFlags() & DECFLAG_HW) {
    } else {
        if (mSet->bEnableHwVideoDecode) {
            mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_SW_VIDEO_DECODER, "Switch to software video decoder");
        }
    }

    if (meta->duration > mDuration) {
        mDuration = meta->duration;
    }

    return ret;
}

void SuperMediaPlayer::updateVideoMeta()
{
    mDemuxerService->GetStreamMeta(mCurrentVideoMeta, mCurrentVideoIndex, false);
    auto *meta = (Stream_meta *) (mCurrentVideoMeta.get());

    int with = meta->displayWidth == 0 ? meta->width : meta->displayWidth;
    int height = meta->displayHeight == 0 ? meta->height : meta->displayHeight;
    if (mVideoWidth != with || mVideoHeight != height || mVideoRotation != meta->rotate) {
        mVideoWidth = with;
        mVideoHeight = height;
        mVideoRotation = meta->rotate;
        mPNotifier->NotifyVideoSizeChanged(mVideoWidth, mVideoHeight);
    }
}

bool SuperMediaPlayer::CreateVideoRender(uint64_t flags)
{
    if (mAVDeviceManager->isVideoRenderValid() && mAVDeviceManager->getVideoRender()->getFlags() == flags) {
        return true;
    }

    // lock mAppStatusMutex before mCreateMutex
    std::lock_guard<std::mutex> uMutex(mCreateMutex);
    mAVDeviceManager->createVideoRender(flags);
    if (!mAVDeviceManager->getVideoRender()) {
        return false;
    }
    mAVDeviceManager->getVideoRender()->setScale(convertScaleMode(mSet->scaleMode));
    mAVDeviceManager->getVideoRender()->setRotate(convertRotateMode(mSet->rotateMode));
    mAVDeviceManager->getVideoRender()->setBackgroundColor(mSet->mVideoBackgroundColor);
    mAVDeviceManager->getVideoRender()->setFlip(convertMirrorMode(mSet->mirrorMode));
    mAVDeviceManager->getVideoRender()->setDisPlay(mSet->mView);
    mAVDeviceManager->setVideoRenderListener(mVideoRenderListener.get());
    int renderRet = mAVDeviceManager->getVideoRender()->init();

    if (renderRet != 0) {
        // for windows init failed, which may need change render type in future.
        mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_VIDEO_RENDER_INIT_ERROR, "init video render failed");
    }

    mAVDeviceManager->setSpeed(mSet->rate);
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
    if (!mSet->bLowLatency) {
        mSet->bLowLatency = mDemuxerService->getDemuxerHandle()->isLowLatency();
    }

    if (mSet->bLowLatency) {
        decFlag |= DECFLAG_OUTPUT_FRAME_ASAP;
    }

    void *view = nullptr;
    mAVDeviceManager->flushVideoRender();

    if (bHW) {
        if (mAVDeviceManager->isVideoRenderValid() && mAVDeviceManager->getVideoRender()->getFlags() & IVideoRender::FLAG_DUMMY) {
            view = mSet->mView;
            decFlag |= DECFLAG_DIRECT;
        } else {
            if (mAVDeviceManager->isVideoRenderValid()) {
                /*
                 * Get a cached surface, then the mediaCodec video decoder can be reuse,
                 * otherwise the mediaCodec video decoder will be recreate on setUpDecoder.
                 */
                view = mAVDeviceManager->getVideoRender()->getSurface(false);
            }
        }
    }

    if (!mSet->bLowLatency) {
        mSet->bLowLatency = mDemuxerService->getDemuxerHandle()->isLowLatency();
    }

    if (mSet->bLowLatency) {
        decFlag |= DECFLAG_OUTPUT_FRAME_ASAP;
    }
    uint32_t dstFormat = 0;
#ifdef __APPLE__
    dstFormat = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
    if (mFrameCb && mSet->pixelBufferOutputFormat) {
        dstFormat = mSet->pixelBufferOutputFormat;
    } else {
        // TODO: move to VTB decoder
        /*
         *  must set hdr video to output x420, otherwise vtb will output a p420 format
         *  if iOS version little than 14 that can't be displayed by AVSampleBufferDisplayLayer
         */
        if (meta.pixel_fmt == AF_PIX_FMT_YUV420P10BE || meta.pixel_fmt == AF_PIX_FMT_YUV420P10LE) {
            dstFormat = kCVPixelFormatType_420YpCbCr10BiPlanarVideoRange;
        }
    }
#endif
    ret = mAVDeviceManager->setUpDecoder(decFlag, (const Stream_meta *) (&meta), view, SMPAVDeviceManager::DEVICE_TYPE_VIDEO, dstFormat);
    if (ret < 0) {
        return ret;
    }
    {
        std::lock_guard<std::mutex> lock(mAppStatusMutex);
        mMsgCtrlListener->ProcessVideoHoldMsg(mAppStatus == APP_BACKGROUND);
    }
    return ret;
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
    mFirstReadPacketSucMS = 0;
    mCanceled = false;
    mPNotifier->Enable(true);
    FlushSubtitleInfo();
    mSubtitleShowedQueue.clear();
    mSubPlayer = nullptr;
    mBSReadCb = nullptr;
    mBSCbArg = nullptr;
    mBSSeekCb = nullptr;
    mBSCbArg = nullptr;
    mUtil->reset();
    mDcaManager->reset();
    mVideoInterlaced = InterlacedType_UNKNOWN;
    mVideoParserTimes = 0;
    mVideoPtsRevert = mAudioPtsRevert = false;
    mLowMem = false;
    mCurrentVideoMeta = nullptr;
    mAdaptiveVideo = false;
    dropLateVideoFrames = false;
    mVideoCatchingUp = false;
    mBRendingStart = false;
    mSubtitleEOS = false;
    mSecretPlayBack = false;
    mDrmKeyValid = false;
    mPtsDiscontinueDelta = INT64_MIN;
    mCurrentPos = 0;
    mCATimeBase = 0;
    mWATimeBase = 0;
    mSuggestedPresentationDelay = 0;
    mLiveTimeSyncType = LiveTimeSyncType::LiveTimeSyncNormal;
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
        std::deque<StreamInfo *> &streamInfoQueue = mMediaInfo.mStreamInfoQueue;
        for (StreamInfo *info : streamInfoQueue) {
            if (info->streamIndex == streamIndex) {
                return info;
            }
        }
    }

    return nullptr;
}

void SuperMediaPlayer::RenderCallback(StreamType type, bool rendered, IAFFrame::AFFrameInfo &info)
{
    //   AF_LOGD("video stream render pts is %lld", pts);
    if (mCanceled) {
        return;
    }

    if ((PLAYER_PREPARED != mPlayStatus) && (PLAYER_PAUSED != mPlayStatus) && (PLAYER_PLAYING != mPlayStatus)) {
        return;
    }

    MsgParam param;
    param.renderedParam.info = info;
    param.renderedParam.rendered = rendered;
    param.renderedParam.type = type;
    param.renderedParam.timeMs = af_getsteady_ms();
    putMsg(MSG_INTERNAL_RENDERED, param, false);
}

void SuperMediaPlayer::checkFirstRender()
{
    if (!mFirstRendered) {
        mFirstRendered = true;
        AF_LOGI("Player NotifyFirstFrame");
        mPNotifier->NotifyFirstFrame();
    }
}

void SuperMediaPlayer::ChangePlayerStatus(PlayerStatus newStatus)
{
    mOldPlayStatus = mPlayStatus;

    if (mPlayStatus != newStatus) {
        mPNotifier->NotifyPlayerStatusChanged(mPlayStatus, newStatus);
        mPlayStatus = newStatus;
    }
}

void SuperMediaPlayer::ResetSeekStatus()
{
    mSeekPos = INT64_MIN;
    mSeekNeedCatch = false;
}

void SuperMediaPlayer::notifySeekEndCallback()
{
    mPNotifier->NotifySeekEnd(mSeekInCache);
}

bool SuperMediaPlayer::IsMute() const
{
    return mSet->bMute;
}

int SuperMediaPlayer::SetListener(const playerListener &Listener)
{
    mSet->mPlayerListener = Listener;

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

int SuperMediaPlayer::setStreamDelay(int index, int64_t time)
{
    if (!(index & EXT_STREAM_BASE) || !mSubPlayer) {
        AF_LOGE("setStreamDelay support ext subtitle only for now\n");
        return -ENOSYS;
    }
    if (mSubPlayer) {
        mSubPlayer->setDelayTime(index, time * 1000);
        mSubPlayer->seek(std::max(getCurrentPosition() + time * 1000, (int64_t) 0));
    }
    return 0;
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
    mAVDeviceManager->pauseAudioRender(!start);
}
void SuperMediaPlayer::SetOnRenderCallBack(onRenderFrame cb, void *userData)
{
    mFrameCb = cb;
    mFrameCbUserData = userData;
}

void SuperMediaPlayer::SetAudioRenderingCallBack(onRenderFrame cb, void *userData)
{
    mAudioRenderingCb = cb;
    mAudioRenderingCbUserData = userData;
}

void SuperMediaPlayer::SetVideoRenderingCallBack(videoRenderingFrameCB cb, void *userData)
{
    mVideoRenderingCb = cb;
    mVideoRenderingCbUserData = userData;
}

void SuperMediaPlayer::SetUpdateViewCB(UpdateViewCB cb, void *userData)
{
    mUpdateViewCB = cb;
    mUpdateViewCBUserData = userData;
}

int SuperMediaPlayer::invokeComponent(std::string content)
{
    return mDcaManager->invoke(content);
}

void SuperMediaPlayer::setDrmRequestCallback(const std::function<DrmResponseData *(const DrmRequestParam &drmRequestParam)> &drmCallback)
{
    mAVDeviceManager->setDrmRequestCallback(drmCallback);
}

void SuperMediaPlayer::ProcessUpdateView()
{

    if (mCurrentVideoIndex < 0) {
        return;
    }

    int videoTag = VideoTag::VIDEO_TAG_NONE;

    updateVideoMeta();
    auto *meta = (Stream_meta *) (mCurrentVideoMeta.get());

    bool isHDR = isHDRVideo(meta);
    if (isHDR) {
        videoTag |= VideoTag::VIDEO_TAG_HDR10;
    }

#ifdef ANDROID
    bool isWideVine = isWideVineVideo(meta);
    if (isWideVine) {
        //TODO set widevine level  by user
        videoTag |= VideoTag::VIDEO_TAG_WIDEVINE_L1;
    }
    if (mUpdateViewCB != nullptr) {
        mUpdateViewCB(videoTag, mUpdateViewCBUserData);
    }
#endif
}

bool SuperMediaPlayer::isWideVineVideo(const Stream_meta *meta)
{
    bool isWideVineVideo = (meta->keyFormat != nullptr && strcmp(meta->keyFormat, "urn:uuid:edef8ba9-79d6-4ace-a3c8-27dcd51d21ed") == 0);
    return isWideVineVideo;
}

bool SuperMediaPlayer::isHDRVideo(const Stream_meta *meta)
{
    bool isHDRVideo = false;

    if (meta->pixel_fmt == AF_PIX_FMT_YUV420P10BE || meta->pixel_fmt == AF_PIX_FMT_YUV420P10LE) {
        AF_LOGI("HDR video\n");
        isHDRVideo = true;
    }
    return isHDRVideo;
}
float SuperMediaPlayer::getCurrentDownloadSpeed()
{
    return mUtil->getCurrentDownloadSpeed();
}

void SuperMediaPlayer::ApsaraAudioRenderCallback::onFrameInfoUpdate(IAFFrame::AFFrameInfo &info, bool rendered)
{
    mPlayer.RenderCallback(ST_TYPE_AUDIO, rendered, info);
}
void SuperMediaPlayer::ApsaraVideoRenderListener::onFrameInfoUpdate(IAFFrame::AFFrameInfo &info, bool rendered)
{
    mPlayer.RenderCallback(ST_TYPE_VIDEO, rendered, info);
}
