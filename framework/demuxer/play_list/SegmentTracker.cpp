//
// Created by moqi on 2018/4/28.
//

#define LOG_TAG "SegmentTracker"

#include "SegmentTracker.h"
#include "Helper.h"
#include "HlsParser.h"
#include "playList_demuxer.h"
#include "utils/timer.h"
#include <algorithm>
#include <data_source/dataSourcePrototype.h>
#include <utility>
#include <utils/errors/framework_error.h>
#include <utils/frame_work_log.h>

#define IS_LIVE (mRep && mRep->b_live)


namespace Cicada {

    SegmentTracker::SegmentTracker(Representation *rep, const IDataSource::SourceConfig &sourceConfig)
        : mRep(rep), mSourceConfig(sourceConfig)
    {
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

    std::shared_ptr<segment> SegmentTracker::getCurSegment()
    {
        std::unique_lock<std::recursive_mutex> locker(mMutex);
        shared_ptr<segment> seg = nullptr;

        if (mRep->GetSegmentList()) {
            seg = mRep->GetSegmentList()->getSegmentByNumber(mCurSegNum);
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
            seg = mRep->GetSegmentList()->getSegmentByNumber(mCurSegNum);
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
        if (segList->hasLHLSSegments()) {
            // lhls stream, liveStartIndex is partial segment index
            auto segments = segList->getSegments();
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
                        break;
                    }
                }
                if (!isFindPart) {
                    // use last independent part
                    auto iter = segments.back();
                    iter->moveToNearestIndependentPart(iter->getSegmentParts().size() - 1);
                    setCurSegNum(iter->getSequenceNumber());
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
                        break;
                    }
                }
                if (!isFindPart) {
                    // use first independent part
                    auto iter = segments.front();
                    iter->moveToNearestIndependentPart(0);
                    setCurSegNum(iter->getSequenceNumber());
                }
            }
        } else {
            // hls stream, liveStartIndex is segment index
            uint64_t curNum;
            if (liveStartIndex >= 0) {
                curNum = std::min(getFirstSegNum() + liveStartIndex, getLastSegNum());
            } else {
                curNum = std::max(getLastSegNum() + liveStartIndex + 1, getFirstSegNum());
            }
            setCurSegNum(curNum);
        }
    }

    int SegmentTracker::loadPlayList()
    {
        int ret;
        string uri;
        string *pUri;

        if (!mRep) {
            return -EINVAL;
        }

        if (mLocation.empty()) {
            std::unique_lock<std::recursive_mutex> locker(mMutex);
            uri = Helper::combinePaths(mRep->getPlaylist()->getPlaylistUrl(),
                                       mRep->getPlaylistUrl());
            pUri = &uri;
        } else {
            pUri = &mLocation;
        }

        AF_LOGD("uri is [%s]\n", pUri->c_str());

        if (mRep->mPlayListType == playList_demuxer::playList_type_hls) {
            if (mPDataSource == nullptr) {
                {
                    std::unique_lock<std::recursive_mutex> locker(mMutex);
                    mPDataSource = dataSourcePrototype::create(*pUri, mOpts);
                    mPDataSource->Set_config(mSourceConfig);
                    mPDataSource->Interrupt(mInterrupted);
                }
                ret = mPDataSource->Open(0);
            } else {
                ret = mPDataSource->Open(*pUri);
            }

            AF_LOGD("ret is %d\n", ret);

            if (ret < 0) {
                AF_LOGE("open url error %s\n", framework_err2_string(ret));
                return ret;
            }

            if (mLocation.empty()) {
                std::string location("location");
                mLocation = mPDataSource->GetOption(location);
            }

            auto *parser = new HlsParser(pUri->c_str());
            dataSourceIO *dio = new dataSourceIO(mPDataSource);
            parser->setDataSourceIO(dio);
            playList *pPlayList = parser->parse(*pUri);

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
                    pList->merge(sList);
                } else {
                    mRep->SetSegmentList(sList);
                }

                rep->SetSegmentList(nullptr);

                // update is live
                mRep->b_live = rep->b_live;

                if (pPlayList->getDuration() > 0) {
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
            }

            // mRep->mStreamType = rep->mStreamType;
            // TODO save parser
            delete parser;
            delete dio;
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

    int64_t SegmentTracker::getTargetDuration()
    {
        if (mRep->GetSegmentList() != nullptr) {
            return mRep->GetSegmentList()->getTargetDuration();
        } else {
            return mRep->targetDuration;
        }
    }
}
