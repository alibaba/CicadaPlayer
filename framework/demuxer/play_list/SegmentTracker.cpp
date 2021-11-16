//
// Created by moqi on 2018/4/28.
//

#define LOG_TAG "SegmentTracker"

#include "SegmentTracker.h"
#include "AdaptationSet.h"
#include "Helper.h"
#include "HlsParser.h"
#include "Period.h"
#include "Representation.h"
#include "SegmentList.h"
#include "data_source/dataSourcePrototype.h"
#include "playList_demuxer.h"
#include "utils/errors/framework_error.h"
#include "utils/frame_work_log.h"
#include "utils/timer.h"
#include <algorithm>
#include <cassert>
#include <utility>

#define IS_LIVE (mRep && mRep->b_live)


namespace Cicada {

    SegmentTracker::SegmentTracker(Representation *rep, const IDataSource::SourceConfig &sourceConfig)
        : mRep(rep), mSourceConfig(sourceConfig)
    {
        mCanBlockReload = mRep->mCanBlockReload;
        if (mCanBlockReload && mTargetDuration > 0) {
            mSourceConfig.connect_time_out_ms = 3 * mTargetDuration;
        }
        mCanSkipUntil = mRep->mCanSkipUntil;
        mThread = NEW_AF_THREAD(threadFunction);
    }

    SegmentTracker::~SegmentTracker()
    {
        {
            std::unique_lock<std::mutex> locker(mSegMutex);
            mStopLoading = true;
            mNeedUpdate = true;
        }
        mSegCondition.notify_all();
        delete mThread;
        std::unique_lock<std::recursive_mutex> locker(mMutex);

        if (playListOwnedByMe) {
            delete mPPlayList;
        }

        if (mPDataSource) {
            mPDataSource->Interrupt(true);
            mPDataSource->Close();
            delete mPDataSource;
        }
    }

    std::shared_ptr<segment> SegmentTracker::getCurSegment(bool force)
    {
        std::unique_lock<std::recursive_mutex> locker(mMutex);
        shared_ptr<segment> seg = nullptr;

        if (mRep->GetSegmentList()) {
            seg = mRep->GetSegmentList()->getSegmentByNumber(mCurSegNum, force);
        }

        if (seg) {
            mCurSegNum = seg->getSequenceNumber();
        }

        return seg;
    }

    std::shared_ptr<segment> SegmentTracker::getNextSegment()
    {
        std::unique_lock<std::recursive_mutex> locker(mMutex);
        shared_ptr<segment> seg = nullptr;
        mCurSegNum++;

        if (mRep->GetSegmentList()) {
            seg = mRep->GetSegmentList()->getSegmentByNumber(mCurSegNum, true);
        }

        if (seg) {
            mCurSegNum = seg->getSequenceNumber();
        } else {
            mCurSegNum--;
        }
        
        return seg;
    }

    int SegmentTracker::GetRemainSegmentCount()
    {
        std::unique_lock<std::recursive_mutex> locker(mMutex);
        int count = -1;

        if (mRep->GetSegmentList()) {
            count = mRep->GetSegmentList()->getRemainSegmentAfterNumber(mCurSegNum);
        }

        return count;
    }

    void SegmentTracker::MoveToLiveStartSegment(const int64_t liveStartIndex)
    {
        SegmentList *segList = mRep->GetSegmentList();
        if (segList == nullptr) {
            AF_LOGW("SegmentTracker::MoveToLiveStartSegment, segmentList is empty");
            return;
        }
        auto segments = segList->getSegments();
        if (segList->hasLHLSSegments()) {
            if (mRep->mPartHoldBack > 0.0) {
                double duration = 0.0;
                bool isFindPart = false;
                for (auto iter = segments.rbegin(); iter != segments.rend(); iter++) {
                    const vector<SegmentPart> &segmentParts = (*iter)->getSegmentParts();
                    if (segmentParts.size() > 0) {
                        for (int i = segmentParts.size() - 1; i >= 0; i--) {
                            duration += segmentParts[i].duration / 1000000.0f;
                            if (duration >= mRep->mPartHoldBack) {
                                (*iter)->moveToNearestIndependentPart(i);
                                isFindPart = true;
                                setCurSegNum((*iter)->getSequenceNumber());
                                std::string segUrl = (*iter)->getDownloadUrl();
                                AF_LOGI("SegmentTracker::MoveToLiveStartSegment, segUrl=%s", segUrl.c_str());
                                break;
                            }
                        }
                        if (isFindPart) {
                            break;
                        }
                    } else {
                        duration += (*iter)->duration / 1000000.0f;
                        if (duration >= mRep->mPartHoldBack) {
                            isFindPart = true;
                            setCurSegNum((*iter)->getSequenceNumber());
                            std::string segUrl = (*iter)->getDownloadUrl();
                            AF_LOGI("SegmentTracker::MoveToLiveStartSegment, segUrl=%s", segUrl.c_str());
                            break;
                        }
                    }
                }
                if (!isFindPart) {
                    // use first independent part
                    auto iter = segments.front();
                    iter->moveToNearestIndependentPart(0);
                    setCurSegNum(iter->getSequenceNumber());
                    std::string segUrl = iter->getDownloadUrl();
                    AF_LOGI("SegmentTracker::MoveToLiveStartSegment, segUrl=%s", segUrl.c_str());
                }
            } else {
                // playlist has no PART-HOLD-BACK, use liveStartIndex , liveStartIndex is partial segment index
                if (liveStartIndex >= 0) {
                    int offset = liveStartIndex;
                    bool isFindPart = false;
                    for (auto iter = segments.begin(); iter != segments.end(); iter++) {
                        const vector<SegmentPart> &segmentParts = (*iter)->getSegmentParts();
                        if (offset >= segmentParts.size()) {
                            offset -= segmentParts.size();
                        } else {
                            (*iter)->moveToNearestIndependentPart(offset);
                            isFindPart = true;
                            setCurSegNum((*iter)->getSequenceNumber());
                            std::string segUrl = (*iter)->getDownloadUrl();
                            AF_LOGI("SegmentTracker::MoveToLiveStartSegment, segUrl=%s", segUrl.c_str());
                            break;
                        }
                    }
                    if (!isFindPart) {
                        // use last independent part
                        auto iter = segments.back();
                        iter->moveToNearestIndependentPart(iter->getSegmentParts().size() - 1);
                        setCurSegNum(iter->getSequenceNumber());
                        std::string segUrl = iter->getDownloadUrl();
                        AF_LOGI("SegmentTracker::MoveToLiveStartSegment, segUrl=%s", segUrl.c_str());
                    }
                } else {
                    int offset = -liveStartIndex - 1;
                    bool isFindPart = false;
                    for (auto iter = segments.rbegin(); iter != segments.rend(); iter++) {
                        const vector<SegmentPart> &segmentParts = (*iter)->getSegmentParts();
                        if (offset >= segmentParts.size()) {
                            offset -= segmentParts.size();
                        } else {
                            (*iter)->moveToNearestIndependentPart(segmentParts.size() - 1 - offset);
                            isFindPart = true;
                            setCurSegNum((*iter)->getSequenceNumber());
                            std::string segUrl = (*iter)->getDownloadUrl();
                            AF_LOGI("SegmentTracker::MoveToLiveStartSegment, segUrl=%s", segUrl.c_str());
                            break;
                        }
                    }
                    if (!isFindPart) {
                        // use first independent part
                        auto iter = segments.front();
                        iter->moveToNearestIndependentPart(0);
                        setCurSegNum(iter->getSequenceNumber());
                        std::string segUrl = iter->getDownloadUrl();
                        AF_LOGI("SegmentTracker::MoveToLiveStartSegment, segUrl=%s", segUrl.c_str());
                    }
                }
            }
        } else {
            uint64_t curNum = 0;
            if (mRep->mHoldBack > 0.0) {
                double duration = 0.0;
                bool isFind = false;
                for (auto iter = segments.rbegin(); iter != segments.rend(); iter++) {
                    duration += (*iter)->duration / 1000000.0f;
                    if (duration >= mRep->mHoldBack) {
                        isFind = true;
                        curNum = (*iter)->getSequenceNumber();
                        setCurSegNum(curNum);
                        AF_LOGI("SegmentTracker::MoveToLiveStartSegment, seg num=%llu", curNum);
                        break;
                    }
                }
                if (!isFind) {
                    // use first segment
                    auto iter = segments.front();
                    curNum = iter->getSequenceNumber();
                    setCurSegNum(curNum);
                    AF_LOGI("SegmentTracker::MoveToLiveStartSegment, seg num=%llu", curNum);
                }
            } else {
                // playlist has no HOLD-BACK, use liveStartIndex , liveStartIndex is segment index
                if (liveStartIndex >= 0) {
                    curNum = std::min(getFirstSegNum() + liveStartIndex, getLastSegNum());
                } else {
                    curNum = std::max(getLastSegNum() + liveStartIndex + 1, getFirstSegNum());
                }
                setCurSegNum(curNum);
                AF_LOGI("SegmentTracker::MoveToLiveStartSegment, seg num=%llu", curNum);
            }
        }
    }

    int SegmentTracker::loadPlayList(bool noSkip)
    {
        int ret;
        string uri;
        bool useSkip = false;

        if (!mRep) {
            return -EINVAL;
        }

        {
            std::unique_lock<std::recursive_mutex> locker(mMutex);
            if (mLocation.empty()) {
                uri = Helper::combinePaths(mRep->getPlaylist()->getPlaylistUrl(), mRep->getPlaylistUrl());
            } else {
                uri = mLocation;
            }
            if (mCanBlockReload && mCurrentMsn >= 0) {
                if (uri.find('?') == std::string::npos) {
                    uri += "?";
                } else {
                    uri += "&";
                }
                uri += "_HLS_msn=";
                uri += std::to_string(mCurrentMsn);
                uri += "&_HLS_part=";
                uri += std::to_string(mCurrentPart);
            }
            if (!noSkip && mCanSkipUntil > 0.0 && af_getsteady_ms() - mLastPlaylistUpdateTime < mCanSkipUntil * 0.5 * 1000) {
                if (uri.find('?') == std::string::npos) {
                    uri += "?";
                } else {
                    uri += "&";
                }
                uri += "_HLS_skip=YES";
                useSkip = true;
            }
        }

        AF_LOGD("loadPlayList uri is [%s]\n", uri.c_str());

        if (mRep->mPlayListType == playList_demuxer::playList_type_hls) {
            mLoadingPlaylist = true;
            if (mExtDataSource) {
                ret = mExtDataSource->Open(uri);
            } else {
                if (mPDataSource == nullptr) {
                    {
                        std::unique_lock<std::recursive_mutex> locker(mMutex);
                        mPDataSource = dataSourcePrototype::create(uri, mOpts);
                        mPDataSource->Set_config(mSourceConfig);
                        mPDataSource->Interrupt(mInterrupted);
                    }
                    ret = mPDataSource->Open(0);
                } else {
                    ret = mPDataSource->Open(uri);
                }
            }
            mLoadingPlaylist = false;

            AF_LOGD("loadPlayList ret is %d\n", ret);

            if (ret < 0) {
                AF_LOGE("open url error %s\n", framework_err2_string(ret));
                if (ret == gen_framework_http_errno(404)) {
                    if (mReloadErrorStartTime == INT64_MIN) {
                        mReloadErrorStartTime = af_getsteady_ms();
                    } else {
                        if (af_getsteady_ms() - mReloadErrorStartTime > mSourceConfig.low_speed_time_ms) {
                            return -EIO;
                        }
                    }
                }
                return ret;
            } else {
                mReloadErrorStartTime = INT64_MIN;
            }

            if (mLocation.empty()) {
                std::string location("location");
                if (mExtDataSource) {
                    mLocation = mExtDataSource->GetOption(location);
                } else {
                    assert(mPDataSource);
                    mLocation = mPDataSource->GetOption(location);
                }
            }

            auto *parser = new HlsParser(uri.c_str());
            if (mExtDataSource) {
                parser->setDataSourceIO(new dataSourceIO(mExtDataSource));
            } else {
                parser->setDataSourceIO(new dataSourceIO(mPDataSource));
            }
            playList *pPlayList = parser->parse(uri);

            //  mPPlayList->dump();
            // mediaPlayList only have one Representation
            if (pPlayList != nullptr) {
                std::unique_lock<std::recursive_mutex> locker(mMutex);
                Representation *rep = (*(*(*pPlayList->GetPeriods().begin())->GetAdaptSets().begin())->getRepresentations().begin());
                SegmentList *sList = rep->GetSegmentList();
                SegmentList *pList = mRep->GetSegmentList();
                mTargetDuration = rep->targetDuration;
                mPartTargetDuration = rep->partTargetDuration;

                //  sList->print();
                //live stream should always keeps the new lists.
                if (pList) {
                    if (useSkip) {
                        uint64_t oldLastSegNum = pList->getLastSeqNum();
                        uint64_t newFirstSegNum = sList->getFirstSeqNum();
                        if (newFirstSegNum > oldLastSegNum + 1) {
                            mNeedReloadWithoutSkip = true;
                            delete pPlayList;
                            delete parser;
                            return 0;
                        }
                    }
                    pList->merge(sList);
                } else {
                    mRep->SetSegmentList(sList);
                }

                SegmentList *currentSegList = mRep->GetSegmentList();
                if (mCanBlockReload) {
                    auto lastSeg = currentSegList->getSegmentByNumber(currentSegList->getLastSeqNum(), false);
                    bool bHasUnusedParts;
                    mCurrentMsn = lastSeg->getSequenceNumber();
                    if (lastSeg->isDownloadComplete(bHasUnusedParts)) {
                        mCurrentMsn++;
                        mCurrentPart = 0;
                    } else {
                        mCurrentPart = lastSeg->getSegmentParts().size();
                    }
                }
                if (mRep->mPreloadHint.used && !mRep->mPreloadHint.uri.empty()) {
                    if (rep->mPreloadHint.uri != mRep->mPreloadHint.uri && !currentSegList->containPartialSegment(mRep->mPreloadHint.uri)) {
                        // TODO: cancel preload
                    }
                    uint64_t preloadSegNum;
                    if (currentSegList->findPartialSegment(mRep->mPreloadHint.uri, preloadSegNum)) {
                        if (mCurSegNum < preloadSegNum) {
                            mCurSegNum = preloadSegNum;
                            AF_LOGD("[lhls] move to preload segment, segNum=%llu, uri=%s", mCurSegNum, mRep->mPreloadHint.uri.c_str());
                        }
                    }
                    auto curSeg = getCurSegment(false);
                    if (curSeg) {
                        curSeg->moveToPreloadSegment(mRep->mPreloadHint.uri);
                    }
                }
                if (!mRep->mPreloadHint.uri.empty() && mRep->mPreloadHint.uri != rep->mPreloadHint.uri) {
                    mRep->mPreloadHint = rep->mPreloadHint;
                }

                rep->SetSegmentList(nullptr);

                // update is live
                mRep->b_live = rep->b_live;

                if (pPlayList->getDuration() > 0 && mPDataSource) {
                    mPDataSource->Close();
                    delete mPDataSource;
                    mPDataSource = nullptr;
                }

                if (mPPlayList == nullptr) { //save the first playList
                    mPPlayList = pPlayList;
                    playListOwnedByMe = true;
                } else {
                    delete pPlayList;
                }
                mLastPlaylistUpdateTime = af_getsteady_ms();
            }

            // mRep->mStreamType = rep->mStreamType;
            // TODO save parser
            delete parser;
            return 0;
        }

        return 0;
    }

    int SegmentTracker::init()
    {
        int ret = 0;

        if (!mInited) {
            SegmentList *list = nullptr;
            {
                std::unique_lock<std::recursive_mutex> locker(mMutex);
                list = mRep->GetSegmentList();
            }

            if (list == nullptr) {  //masterPlayList
                ret = loadPlayList();
                mLastLoadTime = af_gettime_relative();

                if (ret < 0) {
                    AF_LOGE("loadPlayList error %d\n", ret);
                    return ret;
                }
            } else {
                std::unique_lock<std::recursive_mutex> locker(mMutex);
                mPPlayList = mRep->getPlaylist();
                playListOwnedByMe = false;
            }

            if (mRep != nullptr && mRep->GetSegmentList() != nullptr) {
                mRealtime = mRep->GetSegmentList()->hasLHLSSegments();
            }
            
            if (IS_LIVE) {
                mThread->start();
            }

            mInited = true;
        }

        // start from a num
        if (mCurSegNum == 0) {
            std::unique_lock<std::recursive_mutex> locker(mMutex);
            mCurSegNum = mRep->GetSegmentList()->getFirstSeqNum();
        }

        if (mCurSegPos > 0) {
            AF_LOGD("%d mCurSegNum = %llu , mCurSegPos = %llu \n", __LINE__, mCurSegNum,
                    mCurSegPos);
            mCurSegNum = mRep->GetSegmentList()->getFirstSeqNum() + mCurSegPos;
            AF_LOGD("%d mCurSegNum = %llu\n", __LINE__, mCurSegNum);
            mCurSegPos = 0;
        }

        return ret;
    }

    int SegmentTracker::getStreamType() const
    {
        return mRep->mStreamType;
    }

    const string SegmentTracker::getBaseUri()
    {
        std::unique_lock<std::recursive_mutex> locker(mMutex);
        return Helper::combinePaths(mRep->getPlaylist()->getPlaylistUrl(),
                                    mRep->getBaseUrl());
    }

    void SegmentTracker::print()
    {
        AF_LOGD("playList url is %s\n", mRep->getPlaylistUrl().c_str());
        AF_LOGD("BaseUrl url is %s\n", mRep->getBaseUrl().c_str());
        AF_LOGD("getPlaylist()->getPlaylistUrl url is %s\n",
                mRep->getPlaylist()->getPlaylistUrl().c_str());
        mRep->GetSegmentList()->print();
    }

    int SegmentTracker::getStreamInfo(int *width, int *height, uint64_t *bandwidth,
                                      std::string &language)
    {
        return mRep->getStreamInfo(width, height, bandwidth, language);
    }

    string SegmentTracker::getDescriptionInfo()
    {
        return mRep->getAdaptationSet()->getDescription();
    }

    bool SegmentTracker::getSegmentNumberByTime(uint64_t &time, uint64_t &num)
    {
        std::unique_lock<std::recursive_mutex> locker(mMutex);

        if (mRep->GetSegmentList()) {
            return mRep->GetSegmentList()->getSegmentNumberByTime(time, num);
        }

        return false;
    }

    int64_t SegmentTracker::getDuration()
    {
        std::unique_lock<std::recursive_mutex> locker(mMutex);

        if (mPPlayList) {
            return mPPlayList->getDuration();
        }

        return INT64_MIN;
    }

    int SegmentTracker::reLoadPlayList()
    {
        //   AF_TRACE;
        if (IS_LIVE) {

            if (mCanBlockReload) {
                std::unique_lock<std::mutex> locker(mSegMutex);
                if (!mLoadingPlaylist) {
                    mNeedUpdate = true;
                    mSegCondition.notify_all();
                }
            } else {
                int64_t time = af_gettime_relative();

                //   AF_LOGD("mTargetDuration is %lld", (int64_t)mTargetDuration);
                int64_t reloadInterval = 0;
                if (mPartTargetDuration > 0) {
                    // lhls reload interval is 2 times part target duration
                    reloadInterval = mPartTargetDuration * 2;
                } else {
                    // hls reload interval is half target dutaion
                    reloadInterval = mTargetDuration / 2;
                }
                if (time - mLastLoadTime > reloadInterval) {
                    std::unique_lock<std::mutex> locker(mSegMutex);
                    mNeedUpdate = true;
                    mSegCondition.notify_all();
                    mLastLoadTime = time;
                }
            }

            return mPlayListStatus;
        }

        return 0;
    }

    uint64_t SegmentTracker::getSegSize()
    {
        std::unique_lock<std::recursive_mutex> locker(mMutex);
        return mRep->GetSegmentList()->getSegments().size();
    }

    int SegmentTracker::threadFunction()
    {
        // TODO: stop when eos
        while (!mStopLoading) {
            {
                std::unique_lock<std::mutex> locker(mSegMutex);
                mSegCondition.wait(locker, [this]() {
                    return mNeedUpdate.load();
                });
            }

            if (!mStopLoading) {
                mPlayListStatus = loadPlayList();
                if (mNeedReloadWithoutSkip) {
                    mPlayListStatus = loadPlayList(true);
                    mNeedReloadWithoutSkip = false;
                }
                if (!mRealtime && mRep != nullptr && mRep->GetSegmentList() != nullptr)
                {
                    mRealtime = mRep->GetSegmentList()->hasLHLSSegments();
                }
                
                mNeedUpdate = false;
            }
        }

        return 0;
    }

    void SegmentTracker::interrupt(int inter)
    {
        std::unique_lock<std::recursive_mutex> locker(mMutex);
        mInterrupted = inter;
        if (mExtDataSource) {
            mExtDataSource->Interrupt(inter);
        }

        if (mPDataSource) {
            mPDataSource->Interrupt(inter);
        }
    }

    bool SegmentTracker::isInited()
    {
        return mInited;
    }

    bool SegmentTracker::isLive()
    {
        return IS_LIVE;
    }

    string SegmentTracker::getPlayListUri()
    {
        std::unique_lock<std::recursive_mutex> locker(mMutex);
        return Helper::combinePaths(mRep->getPlaylist()->getPlaylistUrl(),
                                    mRep->getPlaylistUrl());
    }

    uint64_t SegmentTracker::getCurSegPosition()
    {
        if (isInited()) {
            uint64_t firstSegNum = getFirstSegNum();
            uint64_t curSegNum = getCurSegNum();
            uint64_t position = curSegNum - firstSegNum - 1;
            AF_LOGD("1206, getCurSegPosition <--- targetSegNum %llu ， firstSegNum = %llu ,curSegNum = %llu \n", position, firstSegNum, curSegNum);
            return position;
        } else {
            AF_LOGD("1206, getCurSegPosition  %llu\n", mCurSegPos);
            return mCurSegPos;
        }
    };

    void SegmentTracker::setCurSegPosition(uint64_t position)
    {
        mCurSegPos = 0;

        if (isInited()) {
            uint64_t targetSegNum = getFirstSegNum() + position;
            AF_LOGD("1206, setCurSegPosition --> targetSegNum %llu\n", targetSegNum);
            setCurSegNum(targetSegNum);
        } else {
            mCurSegPos = position;
            AF_LOGD("1206, setCurSegPosition  %llu\n", mCurSegPos);
        }
        mSeeked = true;
    }

    uint64_t SegmentTracker::getFirstSegNum()
    {
        return mRep->GetSegmentList()->getFirstSeqNum();
    }

    uint64_t SegmentTracker::getLastSegNum()
    {
        return mRep->GetSegmentList()->getLastSeqNum();
    }

    int64_t SegmentTracker::getTargetDuration()
    {
        if (mRep->GetSegmentList() != nullptr) {
            return mRep->GetSegmentList()->getTargetDuration();
        } else {
            return mRep->targetDuration;
        }
    }

    vector<mediaSegmentListEntry> SegmentTracker::getSegmentList()
    {
        vector<mediaSegmentListEntry> ret;
        std::unique_lock<std::recursive_mutex> locker(mMutex);
        if (mRep->GetSegmentList()) {
            auto segments = mRep->GetSegmentList()->getSegments();

            for (auto it = segments.begin(); it != segments.end(); it++) {
                std::string uri = Helper::combinePaths(getBaseUri(), (*it)->getDownloadUrl());
                ret.push_back(mediaSegmentListEntry(uri, (*it)->duration));
            }
        }
        return ret;
    }

    bool SegmentTracker::hasPreloadSegment()
    {
        std::unique_lock<std::recursive_mutex> locker(mMutex);
        if (mRep && mRep->mPreloadHint.isPartialSegment && !mRep->mPreloadHint.uri.empty() && !mRep->mPreloadHint.used) {
            return true;
        }
        return false;
    }

    void SegmentTracker::usePreloadSegment(std::string &uri, int64_t &rangeStart, int64_t &rangeEnd)
    {
        assert(mRep);
        std::unique_lock<std::recursive_mutex> locker(mMutex);
        mRep->mPreloadHint.used = true;
        uri = mRep->mPreloadHint.uri;
        rangeStart = mRep->mPreloadHint.rangeStart;
        rangeEnd = mRep->mPreloadHint.rangeEnd;
    }

    std::shared_ptr<segment> SegmentTracker::usePreloadSegment()
    {
        assert(mRep);
        std::unique_lock<std::recursive_mutex> locker(mMutex);
        mRep->mPreloadHint.used = true;
        std::shared_ptr<segment> ret = std::make_shared<segment>(0);
        ret->setSourceUrl("");
        auto lastSeg = mRep->GetSegmentList()->getSegments().back();
        if (lastSeg->startTime != UINT64_MAX) {
            ret->startTime = lastSeg->startTime + lastSeg->duration;
        }
        if (lastSeg->utcTime >= 0) {
            ret->utcTime = lastSeg->utcTime + lastSeg->duration;
        }
        SegmentPart part;
        part.uri = mRep->mPreloadHint.uri;
        ret->updateParts({part});
        return ret;
    }
    void SegmentTracker::setExtDataSource(IDataSource *source)
    {
        std::unique_lock<std::recursive_mutex> locker(mMutex);
        mExtDataSource = source;
    }
}
