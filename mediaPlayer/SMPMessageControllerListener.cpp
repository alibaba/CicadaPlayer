//
// Created by pingkai on 2020/12/28.
//

#include "SMPMessageControllerListener.h"

#include "SuperMediaPlayer.h"
#include "media_player_error_def.h"
#include <climits>
#include <data_source/dataSourcePrototype.h>
#include <utils/CicadaUtils.h>
#include <utils/af_string.h>
#include <utils/timer.h>

#define HAVE_VIDEO (mPlayer.mCurrentVideoIndex >= 0)
#define HAVE_AUDIO (mPlayer.mCurrentAudioIndex >= 0)
#define HAVE_SUBTITLE (mPlayer.mCurrentSubtitleIndex >= 0)
#define PTS_DISCONTINUE_DELTA (20 * 1000 * 1000)
using namespace Cicada;
SMPMessageControllerListener::SMPMessageControllerListener(SuperMediaPlayer &player) : mPlayer(player)
{}
SMPMessageControllerListener::~SMPMessageControllerListener() = default;

bool SMPMessageControllerListener::OnPlayerMsgIsPadding(PlayMsgType msg, MsgParam msgContent)
{
    bool padding = false;

    switch (msg) {
        case MSG_CHANGE_VIDEO_STREAM:
            padding = mPlayer.mVideoChangedFirstPts != INT64_MIN;
            break;

        case MSG_CHANGE_AUDIO_STREAM:
            padding = mPlayer.mAudioChangedFirstPts != INT64_MIN;
            break;

        case MSG_CHANGE_SUBTITLE_STREAM:
            padding = mPlayer.mSubtitleChangedFirstPts != INT64_MIN;
            break;

        case MSG_SEEKTO:
            if (mPlayer.mSeekFlag) {
                padding = true;
            }

            break;

        default:
            padding = false;
    }

    return padding;
}

void SMPMessageControllerListener::ProcessPrepareMsg()
{
    AF_LOGD("ProcessPrepareMsg start");
    int ret;

    if (mPlayer.mSet->url.empty() && mPlayer.mBSReadCb == nullptr) {
        AF_LOGD("ProcessPrepareMsg url is empty");
        mPlayer.ChangePlayerStatus(PLAYER_ERROR);
        mPlayer.mPNotifier->NotifyError(MEDIA_PLAYER_ERROR_DATASOURCE_EMPTYURL, "Prepare url is empty");
        return;
    }

    if (mPlayer.mPlayStatus != PLAYER_INITIALZED && mPlayer.mPlayStatus != PLAYER_STOPPED) {
        AF_LOGD("ProcessPrepareMsg status is %d", mPlayer.mPlayStatus.load());
        return;
    }

    mPlayer.mPlayStatus = PLAYER_PREPARINIT;
    bool noFile = false;

    if (!(mPlayer.mBSReadCb != nullptr && mPlayer.mBSSeekCb != nullptr && mPlayer.mBSCbArg != nullptr)) {
        if (!mPlayer.mSet->url.empty()) {
            ret = openUrl();

            if (ret < 0) {
                AF_LOGD("%s mDataSource open failed,url is %s %s", __FUNCTION__, mPlayer.mSet->url.c_str(), framework_err2_string(ret));

                if (ret == FRAMEWORK_ERR_EXIT) {
                    // stop by user.
                    //ChangePlayerStatus(PLAYER_STOPPED);
                    return;
                } else if (ret == FRAMEWORK_ERR_PROTOCOL_NOT_SUPPORT) {
                    noFile = true;
                } else {
                    mPlayer.NotifyError(ret);
                    return;
                }
            }
        }
    }

    if (mPlayer.mCanceled) {
        return;
    }

    {
        std::lock_guard<std::mutex> locker(mPlayer.mCreateMutex);
        mPlayer.mDemuxerService = new demuxer_service(mPlayer.mDataSource);
        mPlayer.mDemuxerService->setOptions(&mPlayer.mSet->mOptions);
    }

    std::function<void(std::string, std::string)> demuxerCB = [this](const std::string &key, const std::string &value) -> void {
        this->mPlayer.OnDemuxerCallback(key, value);
    };
    mPlayer.mDemuxerService->setDemuxerCb(demuxerCB);
    mPlayer.mDemuxerService->setNoFile(noFile);

    if (!noFile) {
        mPlayer.mDemuxerService->SetDataCallBack(mPlayer.mBSReadCb, mPlayer.mBSCbArg, mPlayer.mBSSeekCb, mPlayer.mBSCbArg, nullptr);
    }


    //prepare之前seek
    if (mPlayer.mSeekPos > 0) {
        mPlayer.mDemuxerService->Seek(mPlayer.mSeekPos, 0, -1);
        mPlayer.mSeekFlag = true;
    } else {
        mPlayer.ResetSeekStatus();
    }

    AF_LOGD("initOpen start");
    ret = mPlayer.mDemuxerService->createDemuxer((mPlayer.mBSReadCb || noFile) ? demuxer_type_bit_stream : demuxer_type_unknown);

    // TODO: video tool box HW decoder not merge the header
    if (mPlayer.mDemuxerService->getDemuxerHandle()) {
#ifdef __APPLE__
        mPlayer.mDemuxerService->getDemuxerHandle()->setBitStreamFormat(header_type::header_type_extract, header_type::header_type_extract);
#else
        mPlayer.mDemuxerService->getDemuxerHandle()->setBitStreamFormat(header_type::header_type_merge, header_type::header_type_merge);
#endif
        if (noFile) {
            IDataSource::SourceConfig config;
            mPlayer.mDataSource->Get_config(config);
            mPlayer.mDemuxerService->getDemuxerHandle()->setDataSourceConfig(config);
        }

        mPlayer.mDemuxerService->getDemuxerHandle()->SetOption("sessionId", mPlayer.mSet->sessionId);

        mPlayer.mDcaManager->createObservers();
        mPlayer.sendDCAMessage();
    }

    //step2: Demuxer init and getstream index
    ret = mPlayer.mDemuxerService->initOpen((mPlayer.mBSReadCb || noFile) ? demuxer_type_bit_stream : demuxer_type_unknown);

    if (ret < 0) {
        if (ret != FRAMEWORK_ERR_EXIT && !mPlayer.mCanceled) {
            mPlayer.NotifyError(ret);
        }

        return;
    }

    int nbStream = mPlayer.mDemuxerService->GetNbStreams();
    AF_LOGD("Demuxer service get nubmer streams is %d", nbStream);
    unique_ptr<streamMeta> pMeta;
    int bandWidthNearStreamIndex = -1;
    int minBandWidthDelta = INT_MAX;
    int mDefaultBandWidth = mPlayer.mSet->mDefaultBandWidth;

    for (int i = 0; i < nbStream; ++i) {
        mPlayer.mDemuxerService->GetStreamMeta(pMeta, i, false);
        auto *meta = (Stream_meta *) (pMeta.get());

        if (meta->type == STREAM_TYPE_MIXED) {
            mPlayer.mMixMode = true;
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
        mPlayer.mDemuxerService->GetStreamMeta(pMeta, i, false);
        auto *meta = (Stream_meta *) (pMeta.get());

        if (mPlayer.mDuration < 0) {
            mPlayer.mDuration = meta->duration;
        }

        auto *info = new StreamInfo();
        info->streamIndex = i;
        info->subtitleLang = nullptr;
        info->audioLang = nullptr;
        info->description = nullptr;
        AF_LOGD("get a stream %d\n", meta->type);

        if (!mPlayer.mSet->bDisableVideo && meta->type == STREAM_TYPE_VIDEO) {
            info->type = ST_TYPE_VIDEO;
            info->videoWidth = meta->width;
            info->videoHeight = meta->height;
            info->videoBandwidth = (int) meta->bandwidth;
            info->HDRType = VideoHDRType_SDR;
            if (meta->pixel_fmt == AF_PIX_FMT_YUV420P10BE || meta->pixel_fmt == AF_PIX_FMT_YUV420P10LE) {
                info->HDRType = VideoHDRType_HDR10;
            }

            if (meta->description) {
                info->description = strdup((const char *) meta->description);
            }

            mPlayer.mStreamInfoQueue.push_back(info);
            mPlayer.mVideoInterlaced = meta->interlaced;

            if (mPlayer.mCurrentVideoIndex < 0 && !mPlayer.mMixMode && meta->attached_pic == 0) {
                if (bandWidthNearStreamIndex == i) {
                    AF_LOGD("get a video stream\n");
                    openStreamRet = mPlayer.mDemuxerService->OpenStream(i);
                    mPlayer.mCurrentVideoIndex = i;
                    mPlayer.updateVideoMeta();
                    mPlayer.mDemuxerService->GetStreamMeta(mPlayer.mCurrentVideoMeta, i, false);
                }
            }
        } else if (!mPlayer.mSet->bDisableAudio && meta->type == STREAM_TYPE_AUDIO) {
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
            mPlayer.mStreamInfoQueue.push_back(info);

            if (mPlayer.mCurrentAudioIndex < 0 && !mPlayer.mMixMode) {
                AF_LOGD("get a audio stream\n");
                openStreamRet = mPlayer.mDemuxerService->OpenStream(i);
                mPlayer.mCurrentAudioIndex = i;
                mPlayer.mCATimeBase = meta->ptsTimeBase;
            }
        } else if (meta->type == STREAM_TYPE_SUB) {
            info->type = ST_TYPE_SUB;

            if (meta->lang) {
                info->subtitleLang = strdup((const char *) meta->lang);
            }

            if (meta->description) {
                info->description = strdup((const char *) meta->description);
            }

            mPlayer.mStreamInfoQueue.push_back(info);

            if (mPlayer.mCurrentSubtitleIndex < 0 &&
                /*
                 * The codec of the subtitle stream can't be detected in HLS master play list
                 */
                (meta->codec != AF_CODEC_ID_NONE || mPlayer.mDemuxerService->isPlayList())) {
                AF_LOGD("get a subtitle stream\n");
                openStreamRet = mPlayer.mDemuxerService->OpenStream(i);
                mPlayer.mCurrentSubtitleIndex = i;
            }
        } else if (meta->type == STREAM_TYPE_MIXED) {
            info->type = ST_TYPE_VIDEO;
            info->streamIndex = i;
            info->videoBandwidth = (int) meta->bandwidth;
            info->videoWidth = meta->width;
            info->videoHeight = meta->height;
            AF_LOGD("STREAM_TYPE_MIXED bandwidth is %llu", meta->bandwidth);

            if (mPlayer.mMainStreamId >= 0) {
                AF_LOGD("already readed stream");
            } else if (bandWidthNearStreamIndex == i) {
                mPlayer.mMixMode = true;
                openStreamRet = mPlayer.mDemuxerService->OpenStream(i);
                mPlayer.mMainStreamId = i;
            }

            mPlayer.mStreamInfoQueue.push_back(info);
        } else {
            delete info;
        }

        if (openStreamRet < 0) {
            mPlayer.ChangePlayerStatus(PLAYER_ERROR);
            mPlayer.mPNotifier->NotifyError(MEDIA_PLAYER_ERROR_DEMUXER_OPENSTREAM, "open stream failed");
            return;
        }
    }

    if (!HAVE_VIDEO) {
        mPlayer.mSeekNeedCatch = false;
    }

    AF_LOGD("initOpen end");
    mPlayer.mDemuxerService->start();
    mPlayer.ChangePlayerStatus(PLAYER_PREPARING);
    mPlayer.mTimeoutStartTime = INT64_MIN;
}
void SMPMessageControllerListener::ProcessStartMsg()
{
    if (mPlayer.mPlayStatus == PLAYER_PAUSED || mPlayer.mPlayStatus == PLAYER_PREPARED || mPlayer.mPlayStatus == PLAYER_COMPLETION) {
        mPlayer.mUtil->reset();

        if (mPlayer.mPlayStatus != PLAYER_PAUSED) {
            if (HAVE_AUDIO) {
                mPlayer.mMasterClock.setTime(mPlayer.mFirstAudioPts);
            } else {
                mPlayer.mMasterClock.setTime(mPlayer.mFirstVideoPts);
            }
        }

        mPlayer.ChangePlayerStatus(PLAYER_PLAYING);
    }
}

void SMPMessageControllerListener::ProcessPauseMsg()
{
    if (mPlayer.mPlayStatus != PLAYER_PLAYING) {
        return;
    }

    mPlayer.ChangePlayerStatus(PLAYER_PAUSED);
    mPlayer.startRendering(false);
}

// TODO: set layout when init videoRender?
void SMPMessageControllerListener::ProcessSetDisplayMode()
{
    if (mPlayer.mAVDeviceManager->isVideoRenderValid()) {
        mPlayer.mAVDeviceManager->getVideoRender()->setScale(SuperMediaPlayer::convertScaleMode(mPlayer.mSet->scaleMode));
    }
}

void SMPMessageControllerListener::ProcessSetRotationMode()
{
    if (mPlayer.mAVDeviceManager->isVideoRenderValid()) {
        mPlayer.mAVDeviceManager->getVideoRender()->setRotate(SuperMediaPlayer::convertRotateMode(mPlayer.mSet->rotateMode));
    }
}

void SMPMessageControllerListener::ProcessSetMirrorMode()
{
    if (mPlayer.mAVDeviceManager->isVideoRenderValid()) {
        mPlayer.mAVDeviceManager->getVideoRender()->setFlip(SuperMediaPlayer::convertMirrorMode(mPlayer.mSet->mirrorMode));
    }
}

void SMPMessageControllerListener::ProcessSetVideoBackgroundColor()
{
    if (mPlayer.mAVDeviceManager->isVideoRenderValid()) {
        mPlayer.mAVDeviceManager->getVideoRender()->setBackgroundColor(mPlayer.mSet->mVideoBackgroundColor);
    }
}

void SMPMessageControllerListener::ProcessSetViewMsg(void *view)
{
    if (view == mPlayer.mSet->mView) {
        return;
    }

    mPlayer.mSet->mView = view;
    std::unique_lock<std::mutex> uMutex(mPlayer.mCreateMutex);

    if (mPlayer.mAVDeviceManager->getVideoRender() != nullptr) {
        mPlayer.mAVDeviceManager->getVideoRender()->setDisPlay(view);
    }
}

void SMPMessageControllerListener::ProcessSetDataSourceMsg(const std::string &url)
{
    if (mPlayer.mPlayStatus == PLAYER_IDLE || mPlayer.mPlayStatus == PLAYER_STOPPED) {
        mPlayer.mSet->url = url;
        mPlayer.ChangePlayerStatus(PLAYER_INITIALZED);
    }
}

void SMPMessageControllerListener::ProcessSeekToMsg(int64_t seekPos, bool bAccurate)
{
    mPlayer.mSeekNeedCatch = bAccurate;
    mPlayer.mSeekPos = seekPos;

    // seek before prepare, should keep mSeekPos
    if (mPlayer.mPlayStatus < PLAYER_PREPARING ||
        // if reuse player..
        mPlayer.mPlayStatus == PLAYER_STOPPED) {
        return;
    }

    //can seek when finished
    if ((0 >= mPlayer.mDuration) || (mPlayer.mPlayStatus >= PLAYER_STOPPED && mPlayer.mPlayStatus != PLAYER_COMPLETION)) {
        mPlayer.ResetSeekStatus();
        return;
    }
    //checkPosInPackQueue cache in seek
    //TODO: seek sync
    mPlayer.mSeekFlag = true;
    mPlayer.mPlayedVideoPts = INT64_MIN;
    mPlayer.mPlayedAudioPts = INT64_MIN;
    mPlayer.mSoughtVideoPos = INT64_MIN;
    mPlayer.mCurVideoPts = INT64_MIN;
    //flush packet queue
    mPlayer.mSeekInCache = mPlayer.SeekInCache(seekPos);

    mPlayer.mPNotifier->NotifySeeking(mPlayer.mSeekInCache);

    if (mPlayer.mSeekNeedCatch && !HAVE_VIDEO) {
        mPlayer.mSeekNeedCatch = false;
    }

    if (!mPlayer.mSeekInCache) {
        mPlayer.mBufferController->ClearPacket(BUFFER_TYPE_ALL);
        int64_t ret = mPlayer.mDemuxerService->Seek(seekPos, 0, -1);

        if (ret < 0) {
            mPlayer.NotifyError(ret);
        }
        //in case of seekpos larger than duration.
        mPlayer.mPNotifier->NotifyBufferPosition((seekPos <= mPlayer.mDuration ? seekPos : mPlayer.mDuration) / 1000);
        mPlayer.mEof = false;

        if ((mPlayer.mVideoChangedFirstPts != INT64_MAX) && (INT64_MIN != mPlayer.mVideoChangedFirstPts)) {
            mPlayer.mVideoChangedFirstPts = seekPos;
        }
    } else {
        AF_LOGI("sought in cache");

        if (mPlayer.mSeekNeedCatch) {
            int64_t videoPos = mPlayer.mBufferController->GetKeyTimePositionBefore(BUFFER_TYPE_VIDEO, mPlayer.mSeekPos);

            if (videoPos < (mPlayer.mSeekPos - mPlayer.mSet->maxASeekDelta)) {
                // first frame is far away from seek position, don't suppport accurate seek
                mPlayer.mSeekNeedCatch = false;
            } else {
                mPlayer.mBufferController->ClearPacketBeforeTimePos(BUFFER_TYPE_AUDIO, mPlayer.mSeekPos);
            }
        }

        if ((mPlayer.mVideoChangedFirstPts != INT64_MAX) && (INT64_MIN != mPlayer.mVideoChangedFirstPts) &&
            (seekPos > mPlayer.mVideoChangedFirstPts)) {
            mPlayer.mVideoChangedFirstPts = seekPos;
        }
    }

    mPlayer.FlushVideoPath();
    mPlayer.FlushAudioPath();
    mPlayer.FlushSubtitleInfo();

    if (mPlayer.mSubPlayer) {
        mPlayer.mSubPlayer->seek(seekPos);
    }

    mPlayer.mFirstBufferFlag = true;
    mPlayer.mMasterClock.setTime(seekPos);
}

void SMPMessageControllerListener::ProcessMuteMsg()
{
    mPlayer.mAVDeviceManager->setMute(mPlayer.mSet->bMute);
}

void SMPMessageControllerListener::ProcessSwitchStreamMsg(int index)
{
    if (mPlayer.mDemuxerService == nullptr) {
        return;
    }

    Stream_type type = STREAM_TYPE_UNKNOWN;
    int i;
    int number = mPlayer.mDemuxerService->GetNbStreams();

    for (i = 0; i < number; i++) {
        if (index == i) {
            unique_ptr<streamMeta> pMeta;
            mPlayer.mDemuxerService->GetStreamMeta(pMeta, i, false);
            auto *meta = (Stream_meta *) (pMeta.get());
            type = meta->type;
            break;
        }
    }

    if (i >= number) {
        AF_LOGW("no such stream\n");
        return;
    }

    if (mPlayer.mDuration == 0) {
        int id = GEN_STREAM_INDEX(index);

        if (mPlayer.mMainStreamId == -1 || mPlayer.mMainStreamId == id) {
            AF_LOGD("current stream index is the same");
            return;
        }

        mPlayer.mVideoChangedFirstPts = INT64_MAX;
        mPlayer.mAudioChangedFirstPts = INT64_MAX;
        mPlayer.mEof = false;
        mPlayer.mDemuxerService->SwitchStreamAligned(mPlayer.mMainStreamId, id);
        return;
    }

    if (type == STREAM_TYPE_MIXED) {
        int id = GEN_STREAM_INDEX(index);

        if (mPlayer.mMainStreamId == -1 || mPlayer.mMainStreamId == id) {
            AF_LOGD("current stream index is the same");
            return;
        }

        mPlayer.mVideoChangedFirstPts = INT64_MAX;
        mPlayer.mAudioChangedFirstPts = INT64_MAX;
        mPlayer.mEof = false;
        switchVideoStream(id, type);
        return;
    }

    if (type == STREAM_TYPE_SUB && mPlayer.mCurrentSubtitleIndex >= 0 && mPlayer.mCurrentSubtitleIndex != index) {
        return switchSubTitle(index);
    } else if (type == STREAM_TYPE_AUDIO && mPlayer.mCurrentAudioIndex >= 0 && mPlayer.mCurrentAudioIndex != index) {
        return switchAudio(index);
    } else if (type == STREAM_TYPE_VIDEO && mPlayer.mCurrentVideoIndex >= 0 && mPlayer.mCurrentVideoIndex != index) {
        return switchVideoStream(index, type);
    }
}

void SMPMessageControllerListener::ProcessVideoRenderedMsg(int64_t pts, int64_t timeMs, bool rendered, void *picUserData)
{
    mPlayer.mUtil->render(pts);
    if (rendered) {
        mPlayer.checkFirstRender();
    }

    if (!mPlayer.mSeekFlag) {
        mPlayer.mCurVideoPts = pts;
    }

    //AF_LOGD("video stream render pts is %lld ， mVideoChangedFirstPts = %lld ", pts, mVideoChangedFirstPts);

    if ((INT64_MIN != mPlayer.mVideoChangedFirstPts) && (pts >= mPlayer.mVideoChangedFirstPts)) {
        AF_LOGD("video stream changed");
        StreamInfo *info = mPlayer.GetCurrentStreamInfo(ST_TYPE_VIDEO);
        mPlayer.mPNotifier->NotifyStreamChanged(info, ST_TYPE_VIDEO);
        mPlayer.mVideoChangedFirstPts = INT64_MIN;
    }

    mPlayer.mDemuxerService->SetOption("FRAME_RENDERED", pts);

    if (mPlayer.mSet->bEnableVRC) {
        mPlayer.mPNotifier->NotifyVideoRendered(timeMs, pts);
    }

    //TODO packetGotTime
}

void SMPMessageControllerListener::ProcessVideoCleanFrameMsg()
{
    while (!mPlayer.mVideoFrameQue.empty()) {
        int64_t pts = mPlayer.mVideoFrameQue.front()->getInfo().pts;
        ProcessVideoRenderedMsg(pts, af_getsteady_ms(), false, nullptr);
        mPlayer.mVideoFrameQue.front()->setDiscard(true);
        mPlayer.mVideoFrameQue.pop();
    }

    mPlayer.mAVDeviceManager->flushVideoRender();

    mPlayer.mPlayedVideoPts = INT64_MIN;
    mPlayer.mCurVideoPts = INT64_MIN;
    mPlayer.videoDecoderFull = false;
    mPlayer.mVideoPtsRevert = false;
    mPlayer.dropLateVideoFrames = true;
}

void SMPMessageControllerListener::ProcessVideoHoldMsg(bool hold)
{
    if (mPlayer.mAVDeviceManager->getDecoder(SMPAVDeviceManager::DEVICE_TYPE_VIDEO)) {
        mPlayer.mAVDeviceManager->getDecoder(SMPAVDeviceManager::DEVICE_TYPE_VIDEO)->holdOn(hold);

        if (!hold) {
            int size = mPlayer.mAVDeviceManager->getDecoder(SMPAVDeviceManager::DEVICE_TYPE_VIDEO)->getRecoverQueueSize();

            if (size > mPlayer.mSet->maxVideoRecoverSize) {
                string des = "video decoder recover size too large:" + AfString::to_string(size);
                mPlayer.mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_DECODER_RECOVER_SIZE, des.c_str());
            }
        }
    }
}

void SMPMessageControllerListener::ProcessSetSpeed(float speed)
{
    if (!CicadaUtils::isEqual(mPlayer.mSet->rate, speed)) {
        mPlayer.mAVDeviceManager->setSpeed(speed);
        mPlayer.mSet->rate = speed;
        mPlayer.mMasterClock.SetScale(speed);
    }
}

void SMPMessageControllerListener::ProcessAddExtSubtitleMsg(const std::string &url)
{
    lock_guard<mutex> uMutex(mPlayer.mCreateMutex);

    if (mPlayer.mSubPlayer == nullptr) {
        mPlayer.mSubListener = unique_ptr<mediaPlayerSubTitleListener>(new mediaPlayerSubTitleListener(*(mPlayer.mPNotifier)));
        mPlayer.mSubPlayer = unique_ptr<subTitlePlayer>(new subTitlePlayer(*(mPlayer.mSubListener)));
    }

    mPlayer.mSubPlayer->add(url);
}

void SMPMessageControllerListener::ProcessSelectExtSubtitleMsg(int index, bool select)
{
    lock_guard<mutex> uMutex(mPlayer.mCreateMutex);

    if (mPlayer.mSubPlayer == nullptr) {
        AF_LOGE("select ext subtitle error\n");
        mPlayer.mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_SUBTITLE_SELECT_ERROR, "No such subtitle stream");
        return;
    }

    int ret = mPlayer.mSubPlayer->select(index, select);

    if (ret < 0) {
        AF_LOGE("select ext subtitle error\n");
        mPlayer.mPNotifier->NotifyEvent(MEDIA_PLAYER_EVENT_SUBTITLE_SELECT_ERROR, "No such subtitle stream");
    }

    if (select) {
        mPlayer.mSubPlayer->seek(mPlayer.getCurrentPosition());
    }
}


void SMPMessageControllerListener::switchVideoStream(int index, Stream_type type)
{
    int count = (int) mPlayer.mStreamInfoQueue.size();
    StreamInfo *currentInfo = nullptr;
    StreamInfo *willChangeInfo = nullptr;
    int i;
    int currentId = mPlayer.mCurrentVideoIndex;

    if (type == STREAM_TYPE_MIXED) {
        currentId = GEN_STREAM_INDEX(mPlayer.mCurrentVideoIndex);
    }

    for (i = 0; i < count; i++) {
        StreamInfo *info = mPlayer.mStreamInfoQueue[i];

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

    AF_LOGD("video change video bitrate before is %d,after is %d", currentInfo->videoBandwidth, willChangeInfo->videoBandwidth);
    //TODO: different strategy
    mPlayer.mWillChangedVideoStreamIndex = index;
    mPlayer.mVideoChangedFirstPts = INT64_MAX;

    if (willChangeInfo->videoBandwidth < currentInfo->videoBandwidth) {
        mPlayer.mDemuxerService->SwitchStreamAligned(currentId, index);
    } else {
        mPlayer.mMixMode = (type == STREAM_TYPE_MIXED);
        int videoCount = 0;
        int64_t startTime = mPlayer.mBufferController->FindSeamlessPointTimePosition(BUFFER_TYPE_VIDEO, videoCount);

        if (startTime == 0 || videoCount < 40) {
            mPlayer.mWillSwitchVideo = true;
            return;
        }

        if (mPlayer.mMixMode) {
            int64_t startTimeA = mPlayer.mBufferController->FindSeamlessPointTimePosition(BUFFER_TYPE_AUDIO, videoCount);

            if (startTimeA == 0 || videoCount < 40) {
                mPlayer.mWillSwitchVideo = true;
                return;
            }

            startTime = std::max(startTime, startTimeA);
        }
        mPlayer.SwitchVideo(startTime);
    }
}

void SMPMessageControllerListener::switchAudio(int index)
{
    // TODO: use position to seek demuxer ,and drop the late packet
    int ret = mPlayer.mDemuxerService->OpenStream(index);

    if (ret < 0) {
        AF_LOGD("subtitle", "switch audio open stream failed,stream index %d\n", index);
        return;
    }
    std::unique_ptr<streamMeta> meta;
    mPlayer.mDemuxerService->GetStreamMeta(meta, index, true);

    mPlayer.mDemuxerService->CloseStream(mPlayer.mCurrentAudioIndex);
    mPlayer.mAudioChangedFirstPts = INT64_MAX;
    mPlayer.mCurrentAudioIndex = index;
    mPlayer.mCATimeBase = ((Stream_meta *) (*meta))->ptsTimeBase;
    int64_t playTime = mPlayer.mMasterClock.GetTime();
    int64_t pts = playTime - mPlayer.mFirstAudioPts;
    mPlayer.mMasterClock.setReferenceClock(nullptr, nullptr);
    mPlayer.mBufferController->ClearPacket(BUFFER_TYPE_AUDIO);
    mPlayer.mEof = false;
    mPlayer.FlushAudioPath();
    mPlayer.mDemuxerService->Seek(pts, 0, index);
    mPlayer.mPlayedAudioPts = INT64_MIN;
}

void SMPMessageControllerListener::switchSubTitle(int index)
{
    int ret = mPlayer.mDemuxerService->OpenStream(index);

    if (ret < 0) {
        AF_LOGD("subtitle", "switch subtitle open stream failed,stream index %d\n", index);
        return;
    }

    mPlayer.mSubtitleChangedFirstPts = INT64_MAX;
    mPlayer.mDemuxerService->CloseStream(mPlayer.mCurrentSubtitleIndex);
    mPlayer.mCurrentSubtitleIndex = index;
    mPlayer.mBufferController->ClearPacket(BUFFER_TYPE_SUBTITLE);
    mPlayer.mEof = false;
    mPlayer.mSubtitleEOS = false;
    mPlayer.FlushSubtitleInfo();
    mPlayer.mDemuxerService->Seek(mPlayer.getCurrentPosition(), 0, index);
}

int SMPMessageControllerListener::openUrl()
{
    IDataSource::SourceConfig config{};
    config.low_speed_time_ms = mPlayer.mSet->timeout_ms;
    config.low_speed_limit = 1;

    switch (mPlayer.mSet->mIpType) {
        case IpResolveWhatEver:
            config.resolveType = IDataSource::SourceConfig::IpResolveWhatEver;
            break;
        case IpResolveV4:
            config.resolveType = IDataSource::SourceConfig::IpResolveV4;
            break;
        case IpResolveV6:
            config.resolveType = IDataSource::SourceConfig::IpResolveV6;
            break;
    }
    //   config.max_time_ms = mSet->timeout;
    config.connect_time_out_ms = mPlayer.mSet->timeout_ms;
    config.http_proxy = mPlayer.mSet->http_proxy;
    config.refer = mPlayer.mSet->refer;
    config.userAgent = mPlayer.mSet->userAgent;
    config.customHeaders = mPlayer.mSet->customHeaders;
    config.listener = mPlayer.mSourceListener.get();
    mPlayer.mSourceListener->enableRetry();

    if (mPlayer.mCanceled) {
        return FRAMEWORK_ERR_EXIT;
    }

    {
        std::lock_guard<std::mutex> locker(mPlayer.mCreateMutex);
        mPlayer.mDataSource = dataSourcePrototype::create(mPlayer.mSet->url, &(mPlayer.mSet->mOptions));
    }

    if (mPlayer.mDataSource) {
        mPlayer.mDataSource->Set_config(config);
        int ret = mPlayer.mDataSource->Open(0);
        return ret;
    }

    return -1;
}