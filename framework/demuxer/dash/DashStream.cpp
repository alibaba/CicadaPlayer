//
// Created by yuyuan on 2021/03/17.
//
#define LOG_TAG "DashStream"

#include "DashStream.h"
#include "DashSegment.h"
#include "DashSegmentTracker.h"
#include "data_source/dataSourcePrototype.h"
#include "demuxer/DemuxerMeta.h"
#include "demuxer/IDemuxer.h"
#include "demuxer/play_list/Helper.h"
#include "utils/af_string.h"
#include "utils/errors/framework_error.h"
#include "utils/frame_work_log.h"
#include "utils/mediaFrame.h"
#include "utils/timer.h"
#include <algorithm>
#include <cassert>
#include <cstdlib>

// TODO support active and no active mode

using namespace Cicada;

static const int defaultInitSegSize = 1024 * 1024;

DashStream::DashStream(DashSegmentTracker *pTracker, int id) : mPTracker(pTracker), mId(id)
{}

DashStream::~DashStream()
{
    close();
    if (mInitSegBuffer) {
        free(mInitSegBuffer);
    }
    delete mThreadPtr;
    delete mPTracker;
    mStreamStartTimeMap.clear();
}

int DashStream::getStreamType() const
{
    return mPTracker->getStreamType();
}

int DashStream::GetRemainSegmentCount()
{
    return mPTracker->GetRemainSegmentCount();
}

int DashStream::read_callback(void *arg, uint8_t *buffer, int size)
{
    auto *pHandle = static_cast<DashStream *>(arg);
    int ret;

    if (pHandle->mInterrupted) {
        return -EIO;
    }

    if (pHandle->mIsDataEOS) {
        AF_LOGE("%s : %d stream(%d),EOS", __func__, __LINE__, pHandle->mPTracker->getStreamType());
        return 0;
    }

    if (pHandle->mInitSegBuffer) {
        int initSegSize = pHandle->mInitSegSize - pHandle->mInitSegPtr;

        if (initSegSize > 0) {
            int readSize = std::min(initSegSize, size);
            memcpy(buffer, pHandle->mInitSegBuffer + pHandle->mInitSegPtr, readSize);
            pHandle->mInitSegPtr += readSize;
            return readSize;
        }
    }

    ret = pHandle->readSegment(buffer, size);

    if (ret == 0) {
        if (pHandle->mStopOnSegEnd) {
            pHandle->mIsEOS = true;
            return 0;
        }
        if (pHandle->mReopen) {
            return 0;
        }
        int updateRet = pHandle->updateSegment();
        if (updateRet == 0) {
            return pHandle->readSegment(buffer, size);
        } else if (updateRet == -EAGAIN) {
            return 0;
        } else {
            return updateRet;
        }
    }

    return ret;
}

int64_t DashStream::seek_callback(void *arg, int64_t offset, int whence)
{
    auto *pHandle = static_cast<DashStream *>(arg);

    return pHandle->seekSegment(offset, whence);
}

int DashStream::readSegment(const uint8_t *buffer, int size)
{
    int ret = 0;

    if (mExtDataSource) {
        ret = mExtDataSource->Read((void *) buffer, size);
    } else if (mPdataSource) {
        ret = mPdataSource->Read((void *) buffer, (size_t) size);
    }

    return ret;
}

int64_t DashStream::seekSegment(off_t offset, int whence)
{
    int64_t ret;
    if (mExtDataSource) {
        ret = mExtDataSource->Seek(offset, whence);
    } else {
        ret = mPdataSource->Seek(offset, whence);
    }

    return ret;
}

int DashStream::open()
{
    return 0;
}

int DashStream::UpdateInitSection()
{
    std::string uri;
    int ret;
    mInitSegPtr = 0;

    Dash::DashSegment *initSeg = mPTracker->getInitSegment();
    if (initSeg == mCurInitSeg || initSeg == nullptr) {
        return 0;
    }

    uri = initSeg->getUrlSegment().toString(mPTracker->getCurSegNum(), mPTracker->getCurrentRepresentation());
    ret = tryOpenSegment(uri, initSeg->startByte, initSeg->endByte);

    if (ret < 0) {
        return ret;
    }

    mCurInitSeg = initSeg;
    mInitSegSize = defaultInitSegSize;
    if (mCurInitSeg->startByte == INT64_MIN && mCurInitSeg->endByte == INT64_MIN) {
        mInitSegSize = seekSegment(0, SEEK_SIZE);
    } else if (mCurInitSeg->startByte != INT64_MIN && mCurInitSeg->endByte == INT64_MIN) {
        mInitSegSize = seekSegment(0, SEEK_SIZE) - mCurInitSeg->startByte;
    } else if (mCurInitSeg->startByte == INT64_MIN && mCurInitSeg->endByte != INT64_MIN) {
        mInitSegSize = mCurInitSeg->endByte + 1;
    } else {
        mInitSegSize = mCurInitSeg->endByte - mCurInitSeg->startByte + 1;
    }

    if (mInitSegSize < 0) {
        mInitSegSize = defaultInitSegSize;
    }

    if (mInitSegBuffer) {
        free(mInitSegBuffer);
    }

    mInitSegBuffer = static_cast<uint8_t *>(malloc(mInitSegSize));
    int size = 0;

    do {
        // TODO: realloc mInitSegBuffer
        ret = readSegment(mInitSegBuffer + size, mInitSegSize - size);

        if (ret > 0) {
            size += ret;
        } else {
            break;
        }
    } while (size < mInitSegSize);

    mInitSegSize = size;
    return 0;
}

int64_t DashStream::getDurationToStartStream()
{
    return mPTracker->getDurationToStartStream();
}

static inline uint64_t getSize(const uint8_t *data, unsigned int len, unsigned int shift)
{
    uint64_t size(0);
    const uint8_t *dataE(data + len);

    for (; data < dataE; ++data) {
        size = size << shift | *data;
    }

    return size;
};

int DashStream::open_internal()
{
    int ret;
    AF_LOGD("mPTracker type is %d\n", mPTracker->getStreamType());
    ret = mPTracker->init();
    if (mPTracker->isLive()) {
        mSuggestedPresentationDelay = mPTracker->getLiveDelay();
        mStreamStartTime = mPTracker->getStreamStartTime();
    } else {
        mSuggestedPresentationDelay = 0;
        mStreamStartTime = -1;
    }
    AF_LOGD("mSuggestedPresentationDelay=%lld, mStreamStartTime=%lld \n", mSuggestedPresentationDelay, mStreamStartTime);

    if (ret < 0) {
        AF_TRACE;
        return ret;
    }

    if (mSeekPendingUs >= 0) {
        uint64_t num = 0;
        auto usSeeked = (uint64_t) mSeekPendingUs;
        bool b_ret = mPTracker->getSegmentNumberByTime(usSeeked, num);

        if (!b_ret) {
            AF_LOGE("getSegmentNumberByTime error us is %lld\n", mSeekPendingUs);
        } else {
            AF_LOGI("%s:%d stream (%d) usSeeked is %lld seek num is %d\n", __func__, __LINE__, mPTracker->getStreamType(), usSeeked, num);
            mPTracker->setCurSegNum(num);
        }

        mSeekPendingUs = -1;
    }

    //   AF_TRACE;
    //   AF_LOGD("mPTracker->getCurSegNum is %llu", mPTracker->getCurSegNum());

    mStopOnSegEnd = false;
    mCurSeg = nullptr;
    if (!mPTracker->bufferingAvailable()) {
        return -EAGAIN;
    }
    mCurSeg = mPTracker->getStartSegment();
    int trySegmentTimes = 0;

    do {
        trySegmentTimes++;

        if (mCurSeg == nullptr) {
            if (mPTracker->isLive()) {
                AF_TRACE;
                return -EAGAIN;
            } else {
                AF_LOGE("can't find seg %llu\n", mPTracker->getCurSegNum());
                return gen_framework_errno(error_class_format, 0);
            }
        }

        ret = UpdateInitSection();
        if (ret < 0) {
            return ret;
        }

        std::string uri = mCurSeg->getUrlSegment().toString(mPTracker->getCurSegNum(), mPTracker->getCurrentRepresentation());
        AF_LOGD("open uri is %s seq is %llu\n", uri.c_str(), mCurSeg->sequence);
        ret = tryOpenSegment(uri, mCurSeg->startByte, mCurSeg->endByte);

        if (isHttpError(ret)) {
            resetSource();
            if (!mPTracker->bufferingAvailable()) {
                return -EAGAIN;
            }
            mCurSeg = mPTracker->getNextSegment();

            if (trySegmentTimes < 5 && !mInterrupted) {
                af_msleep(20);
                continue;
            }
        }

        if (ret < 0) {
            AF_TRACE;
            mDataSourceError = ret;
            resetSource();
            return ret;
        }
    } while (ret < 0);

    if (mInterrupted) {
        return FRAMEWORK_ERR_EXIT;
    }

    ret = createDemuxer();

    if (ret >= 0) {
        mIsOpened_internal = true;
    } else {
        AF_LOGE("open demuxer error %d\n", ret);
        return ret;
    }

    //     mStatus = status_inited;

    if (mPdataSource) {
        string info = mPdataSource->GetOption("connectInfo");
        openInfoArray.addJSON(CicadaJSONItem(info));
    }

    return ret;
}

int64_t DashStream::getPackedStreamPTS()
{
    return INT64_MIN;
}

int DashStream::createDemuxer()
{
    int ret;
    mError = 0;
    mDataSourceError = 0;

    if (mDemuxerMeta && mDemuxerMeta->id != mCurSeg->discontinuity) {
        mDemuxerMeta = nullptr;
    }

    {
        std::lock_guard<std::mutex> lock(mHLSMutex);
        mPDemuxer = std::unique_ptr<demuxer_service>(new demuxer_service(nullptr));
    }

    mPDemuxer->setOptions(this->mOpts);
    unique_ptr<DemuxerMeta> demuxerMeta = unique_ptr<DemuxerMeta>(new DemuxerMeta());
    demuxerMeta->ownerUrl = mPTracker->getPlayListUri();
    mPDemuxer->setDemuxerMeta(demuxerMeta);
    mPDemuxer->SetDataCallBack(read_callback, this, nullptr, nullptr, nullptr);
    ret = mPDemuxer->createDemuxer(demuxer_type_unknown);

    if (ret < 0) {
        return ret;
    }

    if (mPDemuxer->getDemuxerHandle()) {
        mPDemuxer->getDemuxerHandle()->setBitStreamFormat(this->mMergeVideoHeader, this->mMergerAudioHeader);
    }

    //        if (mDemuxerMeta) {
    //            mPDemuxer->getDemuxerHandle()->setMeta(mDemuxerMeta.get());
    //        }

    ret = mPDemuxer->initOpen();

    if (ret >= 0) {
        int nbStream = mPDemuxer->GetNbStreams();
        AF_LOGI("file have %d streams\n", nbStream);
        bool needUpdateMeta = false;

        if (!mDemuxerMeta) {
            mDemuxerMeta = std::unique_ptr<DemuxerMetaInfo>(new DemuxerMetaInfo());
            mDemuxerMeta->id = mCurSeg ? mCurSeg->discontinuity : 0;
            needUpdateMeta = true;
        }

        unique_ptr<streamMeta> meta = nullptr;

        // open all stream in demuxer
        for (int i = 0; i < nbStream; ++i) {
            mPDemuxer->GetStreamMeta(meta, i, false);
            Stream_type subType = ((Stream_meta *) (*meta))->type;
            Stream_type trackerType = (Stream_type) mPTracker->getStreamType();
            AF_LOGD("sub type is %d\n", subType);
            AF_LOGD("trackerType type is %d\n", trackerType);

            if ((trackerType == STREAM_TYPE_MIXED && subType != STREAM_TYPE_UNKNOWN) || subType == trackerType) {
                AF_LOGW("open stream  index is %d\n", i);
                mPDemuxer->OpenStream(i);
                OpenedStreamIndex = i;
            }

            if (needUpdateMeta) {
                mDemuxerMeta->meta.push_back(move(meta));
            }

            mDemuxerMeta->bContinue = true;
        }

        mPacketFirstPts = getPackedStreamPTS();
    }

    return ret;
}

int DashStream::tryOpenSegment(const string &uri, int64_t start, int64_t end)
{
    AF_LOGI("tryOpenSegment: %s, [%lld,%lld]\n", uri.c_str(), start, end);
    int retryTimes = 0;
    int ret;

    do {
        resetSource();
        ret = openSegment(uri, start, end);
        retryTimes++;

        if (retryTimes > 2) {
            break;
        }

        af_msleep(20);
    } while (isHttpError(ret) && !mInterrupted);

    return ret;
}

int DashStream::openSegment(const string &uri, int64_t start, int64_t end)
{
    int ret;
    int64_t fixEnd = end;
    if (fixEnd != INT64_MIN) {
        fixEnd++;
    }
    if (mExtDataSource) {
        mExtDataSource->setRange(start, fixEnd);
        return mExtDataSource->Open(uri);
    }

    if (mPdataSource == nullptr) {
        recreateSource(uri);
        mPdataSource->setRange(start, fixEnd);
        ret = mPdataSource->Open(0);
    } else {
        mPdataSource->setRange(start, fixEnd);
        ret = mPdataSource->Open(uri);
    }

    return ret;
}

void DashStream::resetSource()
{
    std::lock_guard<std::mutex> lock(mHLSMutex);

    if (mPdataSource) {
        if (!mIsOpened_internal) {
            string info = mPdataSource->GetOption("connectInfo");
            openInfoArray.addJSON(CicadaJSONItem(info));
        }

        //           std::lock_guard<std::mutex> lock(mHLSMutex);
        //            delete mPdataSource;
        //            mPdataSource = nullptr;
    }
}

void DashStream::recreateSource(const string &url)
{
    resetSource();
    std::lock_guard<std::mutex> lock(mHLSMutex);
    mPdataSource = dataSourcePrototype::create(url, mOpts);
    mPdataSource->Set_config(mSourceConfig);
    mPdataSource->Interrupt(mInterrupted);
}

void DashStream::clearDataFrames()
{
    std::unique_lock<std::mutex> locker(mDataMutex);

    while (0 < mQueue.size()) {
        mQueue.pop_front();
    }
}

void DashStream::close()
{
    stop();

    if (mPDemuxer) {
        mPDemuxer->close();
        std::lock_guard<std::mutex> lock(mHLSMutex);
        mPDemuxer = nullptr;
    }

    mIsOpened = false;
    mIsOpened_internal = false;
    openInfoArray.reset();
}

int DashStream::read_thread()
{
    int ret;

    if (mIsOpened && !mIsOpened_internal) {
        ret = open_internal();

        if (ret == -EAGAIN) {
            AF_LOGI("open_internal again\n");
            af_usleep(10000);
            ret = mPTracker->reLoadPlayList();

            if (ret == gen_framework_http_errno(403)) {
                mError = ret;
            }

            return 0;
        } else if (ret < 0) {
            mError = ret;
            af_msleep(10);
            return 0;// continue retry
        }
    }

    {
        std::unique_lock<std::mutex> waitLock(mDataMutex);
        bool waitResult = mWaitCond.wait_for(waitLock, std::chrono::milliseconds(10),
                                             [this]() { return mQueue.size() <= 1 || mInterrupted || mSwitchNeedBreak; });

        if (!waitResult || mInterrupted || mSwitchNeedBreak) {
            return 0;
        }
    }

    unique_ptr<IAFPacket> tmp{};
    int packet_size = read_internal(tmp);

    if ((nullptr != tmp) && (nullptr != tmp->getData()) && (0 < tmp->getSize())) {
        std::unique_lock<std::mutex> waitLock(mDataMutex);
        mQueue.push_back(move(tmp));
    } else if (nullptr != tmp) {
        AF_LOGE("read_thread frame size be set as 0");
        return 0;
    }

    mWaitCond.notify_one();

    if (packet_size == 0) {
        mIsEOS = true;
        return -1;
    } else if (packet_size < 0) {
        if (packet_size == -EAGAIN) {
            //     AF_LOGD("read timed out");
            af_msleep(10);
            return 0;
        }

        if (packet_size != FRAMEWORK_ERR_EXIT) {
            AF_LOGD("read error 0x%4x %s\n", -packet_size, framework_err2_string(packet_size));
            mError = packet_size;
            af_msleep(10);
            return 0;// continue retry
        }
    }

    return 0;
}

int DashStream::read(unique_ptr<IAFPacket> &packet)
{
    int ret;
    packet = nullptr;

    if (mThreadPtr) {
        std::unique_lock<std::mutex> waitLock(mDataMutex);

        if (mLastReadSuccess && (mQueue.empty())) {
            mWaitCond.wait_for(waitLock, std::chrono::milliseconds(1), [this]() { return !mQueue.empty(); });
        }

        if (mQueue.empty()) {
            mLastReadSuccess = false;

            if (mIsEOS) {
                AF_LOGI("return eos\n");
                return 0;
            } else if (mError < 0) {
                ret = mError;
                mError = 0;
                AF_LOGE("return error %d\n", ret);
                return ret;
            } else {
                return -EAGAIN;
            }
        }

        packet = move(mQueue.front());
        mQueue.pop_front();
        ret = static_cast<int>(packet->getSize());
        mWaitCond.notify_one();
        mLastReadSuccess = true;
        return ret;
    } else {
        return read_internal(packet);
    }
}

int DashStream::updateSegment()
{
    if (!mPTracker->bufferingAvailable()) {
        return -EAGAIN;
    }
    Dash::DashSegment *seg = nullptr;
    AF_LOGD("getCurSegNum is %llu\n", mPTracker->getCurSegNum());
    seg = mPTracker->getNextSegment();

    // if current segment time > live delay, discard it
    if (isLive()) {
        int64_t liveDelay = mPTracker->getLiveDelay();
        int64_t segmentDuration = mPTracker->getSegmentDuration();
        while (mPTracker->getMinAheadTime() > liveDelay + segmentDuration) {
            AF_LOGD("discard segment %llu because it is too late", mPTracker->getCurSegNum());
            seg = mPTracker->getNextSegment();
        }
    }

    int ret = 0;
    mCurSeg = nullptr;

    if (seg) {
        do {
            mCurSeg = seg;
            std::string uri = mCurSeg->getUrlSegment().toString(mPTracker->getCurSegNum(), mPTracker->getCurrentRepresentation());
            ret = tryOpenSegment(uri, seg->startByte, seg->endByte);

            if (isHttpError(ret) || isLocalFileError(ret)) {
                resetSource();
                if (!mPTracker->bufferingAvailable()) {
                    return -EAGAIN;
                }
                seg = mPTracker->getNextSegment();

                if (seg) {
                    af_msleep(20);
                    continue;
                } else if (mPTracker->isLive()) {
                    return -EAGAIN;
                } else {
                    //no more seg
                    break;
                }
            }
        } while (isHttpError(ret) || isLocalFileError(ret));

        if (ret < 0) {
            mDataSourceError = ret;

            if (ret != FRAMEWORK_ERR_EXIT) {
                mError = ret;
            }

            resetSource();
            return ret;
        }
        return 0;
    } else {
        AF_LOGE("EOS");
        mIsDataEOS = true;
        return -EAGAIN;
    }

    return -EAGAIN;
}

int DashStream::read_internal(std::unique_ptr<IAFPacket> &packet)
{
    //TODO: move read synMsgRst to class member
    int ret = mPTracker->reLoadPlayList();

    if (ret == gen_framework_http_errno(403)) {
        return ret;
    }

    packet = nullptr;

    ret = mPDemuxer->readPacket(packet);
    //AF_LOGD("mPDemuxer->readPacket ret is %d,pFrame is %p", ret, *pFrame);

    if (ret == -EAGAIN) {
        if (mInterrupted) {
            return ret;
        }
    } else if (ret == FRAMEWORK_ERR_EXIT) {
        AF_LOGW("mPDemuxer->readPacket FRAMEWORK_ERR_EXIT\n");
    }

    if (ret == 0 && mStopOnSegEnd) {
        mIsEOS = true;
        AF_LOGE("mStopOnSegEnd");
        return 0;
    }

    if (ret == gen_framework_errno(error_class_network, network_errno_http_range)) {
        ret = 0;
    }

    if (ret == 0 || mReopen) {
        if (mReopen) {
            AF_LOGD("reopen");
            mReopen = false;
        }

        ret = updateSegment();

        if (mIsDataEOS) {
            return 0;
        }

        if (ret >= 0) {
            UpdateInitSection();
            ret = createDemuxer();

            if (ret >= 0) {
                int nbStream = mPDemuxer->GetNbStreams();
                AF_LOGI("file have %d streams\n", nbStream);
                // open all stream in demuxer
                Stream_meta meta{};

                for (int i = 0; i < nbStream; ++i) {
                    mPDemuxer->GetStreamMeta(&meta, i, false);

                    if (meta.type == mPTracker->getStreamType() ||
                        (mPTracker->getStreamType() == STREAM_TYPE_MIXED && meta.type != STREAM_TYPE_UNKNOWN)) {
                        mPDemuxer->OpenStream(i);
                    }

                    releaseMeta(&meta);
                }

                mPacketFirstPts = getPackedStreamPTS();
            }
        }

        packet = nullptr;
        return -EAGAIN;
    }

    if (ret == -EAGAIN && mPTracker->isLive()) {

        ret = updateSegment();

        if (ret < 0) {
            return ret;
        }

        return -EAGAIN;
    }

    if (packet != nullptr) {
        //  AF_LOGD("read a frame \n");

        if (mPTracker->getStreamType() != STREAM_TYPE_MIXED) {
            packet->getInfo().streamIndex = 0;
        }

        if (mPacketFirstPts != INT64_MIN && packet->getInfo().pts != INT64_MIN) {
            packet->getInfo().pts += mPacketFirstPts;
            packet->getInfo().dts += mPacketFirstPts;
        }

        if (mCurSeg) {
            // mark the seg start time to first seg frame
            AF_LOGD("stream (%d) mark startTime %llu\n", mPTracker->getStreamType(), mCurSeg->fixedStartTime);
            AF_LOGD("stream (%d)pFrame->pts is %lld pos is %lld flags is %d streamIndex is %d\n", mPTracker->getStreamType(),
                    packet->getInfo().pts, packet->getInfo().pos, packet->getInfo().flags, packet->getInfo().streamIndex);

            if (packet->getInfo().flags == 0) {
                AF_LOGE("not a key frame\n");
                _hex_dump(packet->getData(), 16);
            }

            int nbStreams = GetNbStreams();

            for (int i = 0; i < nbStreams; i++) {
                mStreamStartTimeMap[i].timePosition = mCurSeg->fixedStartTime;
                mStreamStartTimeMap[i].seamlessPoint = true;
            }

            mCurSeg = nullptr;
        }

        int streamIndex = packet->getInfo().streamIndex;
        packet->getInfo().seamlessPoint = mStreamStartTimeMap[streamIndex].seamlessPoint;

        if (mStreamStartTimeMap[streamIndex].seamlessPoint) {
            if (packet->getInfo().pts != INT64_MIN) {
                mStreamStartTimeMap[streamIndex].time2ptsDelta = mStreamStartTimeMap[streamIndex].timePosition - packet->getInfo().pts;
            }

            mStreamStartTimeMap[streamIndex].seamlessPoint = false;
        }

        if (packet->getInfo().duration > 0) {
            mStreamStartTimeMap[streamIndex].frameDuration = packet->getInfo().duration;
        }

        if (packet->getInfo().pts == INT64_MIN && mStreamStartTimeMap[streamIndex].lastFramePts != INT64_MIN &&
            mStreamStartTimeMap[streamIndex].frameDuration != INT64_MIN) {
            packet->getInfo().pts = mStreamStartTimeMap[streamIndex].lastFramePts + mStreamStartTimeMap[streamIndex].frameDuration;
        }

        if (packet->getInfo().pts != INT64_MIN && mStreamStartTimeMap[streamIndex].time2ptsDelta != INT64_MIN) {
            packet->getInfo().timePosition = packet->getInfo().pts + mStreamStartTimeMap[streamIndex].time2ptsDelta;
        } else {
            packet->getInfo().timePosition = INT64_MIN;
        }

        if (packet->getInfo().pts != INT64_MIN) {
            mStreamStartTimeMap[streamIndex].lastFramePts = packet->getInfo().pts;
        }

        int64_t timePos = packet->getInfo().timePosition;
        if (timePos == INT64_MIN) {
            timePos = packet->getInfo().pts;
        }
        if (timePos >= 0 && mStreamStartTime >= 0 && mSuggestedPresentationDelay > 0) {
            if (timePos < af_get_utc_time() - mStreamStartTime - mSuggestedPresentationDelay) {
                //AF_LOGD("setDiscard timePos = %lld", timePos);
                packet->setDiscard(true);
            }
        }

    }

    return ret;
}


int DashStream::GetNbStreams() const
{
    if (mPTracker->getStreamType() == STREAM_TYPE_MIXED) {//mediaPlayList
        if (mPDemuxer) {
            return mPDemuxer->GetNbStreams();
        }
    }

    return 1;// masterPlayList only support one es stream per stream url(ext-media-info ext-stream-info) now
}

int DashStream::GetStreamMeta(Stream_meta *meta, int index, bool sub) const
{
    // if not muxed stream ,get the 0 stream default index
    meta->type = (Stream_type) mPTracker->getStreamType();

    if (meta->type != STREAM_TYPE_MIXED) {
        index = OpenedStreamIndex;
    }

    uint64_t bandwidth;
    std::string lang;
    int width;
    int height;
    mPTracker->getStreamInfo(&width, &height, &bandwidth, lang);
    {
        std::lock_guard<std::mutex> lock(mHLSMutex);

        if (mPDemuxer) {
            mPDemuxer->GetStreamMeta(meta, index, sub);
        }
    }

    // meta->type would be override, recover it
    if (!sub) {
        meta->type = (Stream_type) mPTracker->getStreamType();
    }

    if (meta->height == 0) {
        meta->height = height;
        meta->width = width;
    }

    meta->lang = strdup(lang.c_str());
    meta->bandwidth = bandwidth;
    meta->duration = mPTracker->isLive() ? 0 : mPTracker->getDuration();

    if (!mPTracker->getDescriptionInfo().empty()) {
        meta->description = strdup(mPTracker->getDescriptionInfo().c_str());
    }

    meta->suggestedPresentationDelay = mPTracker->getLiveDelay();

    return 0;
}

bool DashStream::isOpened()
{
    return mIsOpened;
}

int DashStream::start()
{
    //        demuxer_msg::StartReq start;
    //        mPProxyService->SendMsg(start, mPDemuxer->GetAddr(), false);
    mIsOpened = true;
    mIsEOS = false;
    mIsDataEOS = false;
    mStopOnSegEnd = false;
    mError = 0;

    if (mThreadPtr == nullptr) {
        mThreadPtr = NEW_AF_THREAD(read_thread);
    }

    mThreadPtr->start();
    return 0;
}

int DashStream::stop()
{
    AF_TRACE;

    if (mThreadPtr) {
        AF_TRACE;
        interrupt_internal(1);
        mWaitCond.notify_one();
        AF_TRACE;
        mThreadPtr->stop();
        AF_TRACE;
        interrupt_internal(mInterrupted);
        AF_TRACE;
    }

    resetSource();
    {
        std::lock_guard<std::mutex> lock(mHLSMutex);

        if (mPdataSource) {
            mPdataSource->Close();
            delete mPdataSource;
            mPdataSource = nullptr;
        }

        if (mSegKeySource) {
            mSegKeySource->Close();
            delete mSegKeySource;
            mSegKeySource = nullptr;
        }

        mIsOpened_internal = false;
    }
    clearDataFrames();
    AF_LOGD("%s\n", __func__);
    return 0;
}

// TODO: deal seek in read_thread
int64_t DashStream::seek(int64_t us, int flags)
{
    (void) flags;
    uint64_t num = 0;
    auto usSought = (uint64_t) us;
    bool reqReOpen = true;
    AF_LOGD("%s:%d stream (%d) seek us is %lld\n", __func__, __LINE__, mPTracker->getStreamType(), us);

    if (!mPTracker->isInited()) {
        mSeekPendingUs = us;
        AF_LOGI("pending seek\n");
        return us;
    }

    //   int ret = mPTracker->init();
    bool b_ret = mPTracker->getSegmentNumberByTime(usSought, num);

    if (!b_ret) {
        AF_LOGE("(%d)getSegmentNumberByTime error us is %lld\n", mPTracker->getStreamType(), us);
        // us's accuracy is ms, so change duration's accuracy to ms
        bool seekOnLast = false;
        if (us >= (mPTracker->getDuration() / 1000 * 1000)) {
            num = mPTracker->getLastSegNum();

            // reopen will -- it
            if (mIsOpened_internal) {
                num++;
            }
            usSought = us;
            seekOnLast = true;
        }

        if (!seekOnLast) {
            if (mPTracker->getStreamType() == STREAM_TYPE_SUB) {
                mIsEOS = false;
                mError = 0;

                if (mThreadPtr) {
                    mThreadPtr->start();
                }
            } else {
                return -1;
            }
        }
    }

    AF_LOGD("%s:%d stream (%d) usSeeked is %lld seek num is %d\n", __func__, __LINE__, mPTracker->getStreamType(), usSought, num);

    if (mPTracker->getStreamType() == STREAM_TYPE_SUB && num == mPTracker->getCurSegNum()) {
        AF_LOGW("only one  subtitle seg");
        reqReOpen = false;
        seek_internal(num, us);
    }

    {
        std::unique_lock<std::mutex> waitLock(mDataMutex);
        mSwitchNeedBreak = true;
    }

    mWaitCond.notify_one();
    interrupt_internal(1);

    if (mThreadPtr) {
        mThreadPtr->pause();
    }

    interrupt_internal(mInterrupted);
    mSwitchNeedBreak = false;
    clearDataFrames();

    if (reqReOpen) {
        resetSource();

        if (mIsOpened_internal) {
            mReopen = true;
        }

        mPTracker->setCurSegNum(num - 1);
    }

    mIsEOS = false;
    mIsDataEOS = false;
    mError = 0;
    if (mDemuxerMeta) {
        mDemuxerMeta->bContinue = false;
    }

    if (mThreadPtr) {
        mThreadPtr->start();
    }

    return usSought;
}

int64_t DashStream::seek_internal(uint64_t segNum, int64_t us)
{
    (void) segNum;

    // subtitle need us to find frame index to read
    //TODO: use flush to flash all
    if (mPDemuxer) {
        mPDemuxer->Seek(us, 0, -1);
        mPDemuxer->flush();
    }

    return 0;
}

uint64_t DashStream::getCurSegNum()
{
    return mPTracker->getCurSegNum();
}

int DashStream::stopOnSegEnd(bool stop)
{
    mStopOnSegEnd = stop;
    return 0;
}

int DashStream::SetCurSegNum(uint64_t num)
{
    return reopenSegment(num, OpenType::SegNum);
}

int DashStream::reopenSegment(uint64_t num, OpenType openType)
{
    {
        std::unique_lock<std::mutex> waitLock(mDataMutex);
        mSwitchNeedBreak = true;
    }

    mWaitCond.notify_one();

    if (mThreadPtr) {
        mThreadPtr->pause();
    }

    mSwitchNeedBreak = false;
    clearDataFrames();
    resetSource();

    if (mIsOpened_internal) {
        mReopen = true;
        num--;
    }

    if (openType == OpenType::SegNum) {
        mPTracker->setCurSegNum(num);
        AF_LOGD("setCurSegNum %llu\n", num);
    } else if (openType == OpenType::SegPosition) {
        mPTracker->setCurSegPosition(num);
        AF_LOGD("setCurSegPosition %llu\n", num);
    }

    seek_internal(num, 0);
    mIsEOS = false;
    mIsDataEOS = false;
    mError = 0;

    if (mThreadPtr) {
        mThreadPtr->start();
    }

    return 0;
}

uint64_t DashStream::getCurSegPosition()
{
    return mPTracker->getCurSegPosition();
};

int DashStream::setCurSegPosition(uint64_t position)
{
    return reopenSegment(position, OpenType::SegPosition);
}

bool DashStream::isLive()
{
    return mPTracker->isLive();
}

int64_t DashStream::getDuration()
{
    return mPTracker->getDuration();
}

int DashStream::getNBStream() const
{
    if (mPDemuxer) {
        return mPDemuxer->GetNbStreams();
    }

    return 1;
}

void DashStream::interrupt(int inter)
{
    {
        std::unique_lock<std::mutex> waitLock(mDataMutex);
        mInterrupted = static_cast<bool>(inter);
    }
    interrupt_internal(inter);
}

void DashStream::interrupt_internal(int inter)
{
    {
        std::lock_guard<std::mutex> lock(mHLSMutex);

        if (mSegKeySource) {
            mSegKeySource->Interrupt(static_cast<bool>(inter));
        }

        if (mPdataSource) {
            mPdataSource->Interrupt(static_cast<bool>(inter));
        }

        if (mExtDataSource) {
            mExtDataSource->Interrupt(static_cast<bool>(inter));
        }
    }
    {
        std::lock_guard<std::mutex> lock(mHLSMutex);

        if (mPDemuxer) {
            mPDemuxer->interrupt(inter);
        }
    }

    if (mPTracker) {
        mPTracker->interrupt(inter);
    }
}

std::string DashStream::GetProperty(const string &key)
{
    if ("openJsonInfo" == key) {
        return openInfoArray.printJSON();
    } else if ("responseInfo" == key) {
        std::lock_guard<std::mutex> lock(mHLSMutex);

        if (mPdataSource) {
            return mPdataSource->GetOption(key);
        }

        return "";
    } else if ("probeInfo" == key) {
        std::lock_guard<std::mutex> lock(mHLSMutex);

        if (mPDemuxer) {
            return mPDemuxer->GetProperty(-1, key);
        }
    }

    return "";
}

bool DashStream::isRealTimeStream()
{
    if (mPTracker != nullptr) {
        return mPTracker->isRealTimeStream();
    } else {
        return false;
    }
}
