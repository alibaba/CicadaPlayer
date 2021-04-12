//
// Created by yuyuan on 2021/03/18.
//

#define LOG_TAG "DashSegmentTracker"

#include "DashSegmentTracker.h"
#include "DashSegment.h"
#include "ISegmentBase.h"
#include "SegmentBase.h"
#include "SegmentList.h"
#include "SegmentTemplate.h"
#include "SegmentTimeline.h"
#include "data_source/dataSourcePrototype.h"
#include "demuxer/play_list/AdaptationSet.h"
#include "demuxer/play_list/Helper.h"
#include "demuxer/play_list/Representation.h"
#include "demuxer/play_list/playList.h"
#include "demuxer/play_list/playList_demuxer.h"
#include "utils/errors/framework_error.h"
#include "utils/frame_work_log.h"
#include "utils/timer.h"
#include <algorithm>
#include <assert.h>
#include <sstream>
#include <utility>


using namespace Cicada;

const static int SAFETY_BUFFERING_EDGE_OFFSET = 1;
const static int SAFETY_EXPURGING_OFFSET = 2;
const static int64_t BUFFERING_LOWEST_LIMIT = 1000000 * 2;


DashSegmentTracker::DashSegmentTracker(AdaptationSet *adapt, Representation *rep, const IDataSource::SourceConfig &sourceConfig)
    : mAdapt(adapt), mRep(rep), mSourceConfig(sourceConfig)
{
    mThread = NEW_AF_THREAD(threadFunction);
    if (mRep == nullptr) {
        mRep = getNextRepresentation(mAdapt, nullptr);
    }
    if (mRep) {
        mPPlayList = mRep->getPlaylist();
        playListOwnedByMe = false;
    }
}

DashSegmentTracker::~DashSegmentTracker()
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

Dash::DashSegment *DashSegmentTracker::getStartSegment()
{
    if (mRep == nullptr) {
        return nullptr;
    }
    uint64_t startNumber = getStartSegmentNumber(mRep);
    if (mCurrentSegNumber < startNumber) {
        mCurrentSegNumber = startNumber;
    }
    Dash::DashSegment *segment = nullptr;
    bool b_gap = false;
    segment = mRep->getNextMediaSegment(mCurrentSegNumber, &mCurrentSegNumber, &b_gap);
    if (segment == nullptr) {
        return nullptr;
    }
    if (b_gap) {
        --mCurrentSegNumber;
    }
    segment->startTime = mRep->getMediaSegmentStartTime(mCurrentSegNumber);
    return segment;
}

Dash::DashSegment *DashSegmentTracker::getNextSegment()
{
    std::unique_lock<std::recursive_mutex> locker(mMutex);

    if (mRep == nullptr) {
        return nullptr;
    }
    Dash::DashSegment *segment = nullptr;
    bool b_gap = false;
    segment = mRep->getNextMediaSegment(mCurrentSegNumber, &mCurrentSegNumber, &b_gap);
    if (segment == nullptr) {
        return nullptr;
    }
    if (b_gap) {
        --mCurrentSegNumber;
    }
    if (segment) {
        ++mCurrentSegNumber;
    }
    segment->startTime = mRep->getMediaSegmentStartTime(mCurrentSegNumber);
    return segment;
}

Dash::DashSegment *DashSegmentTracker::getInitSegment()
{
    auto rep = getNextRepresentation(mAdapt, nullptr);
    if (rep) {
        return rep->getInitSegment();
    }
    return nullptr;
}

Dash::DashSegment *DashSegmentTracker::getIndexSegment()
{
    auto rep = getNextRepresentation(mAdapt, nullptr);
    if (rep) {
        return rep->getIndexSegment();
    }
    return nullptr;
}

int DashSegmentTracker::GetRemainSegmentCount()
{
    std::unique_lock<std::recursive_mutex> locker(mMutex);
    int count = -1;

    return count;
}

int DashSegmentTracker::loadPlayList()
{
    int ret = 0;

    return ret;
}

int DashSegmentTracker::init()
{
    int ret = 0;

    if (!mInited) {
        if (isLive()) {
            mThread->start();
            //if (mPPlayList && mPPlayList->maxSegmentDuration <= 2 * 1000000) {
            //    mRealtime = true;
            //}
        }
        mInited = true;
    }

    return ret;
}

int DashSegmentTracker::getStreamType() const
{
    if (mRep) {
        return mRep->mStreamType;
    }
    return STREAM_TYPE_UNKNOWN;
}

const std::string DashSegmentTracker::getBaseUri()
{
    std::unique_lock<std::recursive_mutex> locker(mMutex);
    if (mRep) {
        return Helper::combinePaths(mRep->getPlaylist()->getPlaylistUrl(), mRep->getBaseUrl());
    }
    return "";
}

void DashSegmentTracker::print()
{
    if (mRep == nullptr) {
        return;
    }
    AF_LOGD("playList url is %s\n", mRep->getPlaylistUrl().c_str());
    AF_LOGD("BaseUrl url is %s\n", mRep->getBaseUrl().c_str());
    AF_LOGD("getPlaylist()->getPlaylistUrl url is %s\n", mRep->getPlaylist()->getPlaylistUrl().c_str());
}

int DashSegmentTracker::getStreamInfo(int *width, int *height, uint64_t *bandwidth, std::string &language)
{
    if (mRep == nullptr) {
        return -1;
    }
    return mRep->getStreamInfo(width, height, bandwidth, language);
}

std::string DashSegmentTracker::getDescriptionInfo()
{
    return mAdapt->getDescription();
}

bool DashSegmentTracker::getSegmentNumberByTime(uint64_t &time, uint64_t &num)
{
    std::unique_lock<std::recursive_mutex> locker(mMutex);
    if (mRep == nullptr) {
        return false;
    }
    return mRep->getSegmentNumberByTime(time, &num);
}

int64_t DashSegmentTracker::getDuration()
{
    std::unique_lock<std::recursive_mutex> locker(mMutex);

    if (mPPlayList) {
        return mPPlayList->getDuration();
    }

    return INT64_MIN;
}

int DashSegmentTracker::reLoadPlayList()
{
    //   AF_TRACE;
    if (isLive()) {
        int64_t time = af_gettime_relative();

        // todo: reload interval
        int64_t reloadInterval = 2;
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

uint64_t DashSegmentTracker::getSegSize()
{
    assert(false && "not implement");
    std::unique_lock<std::recursive_mutex> locker(mMutex);
    return 0;
}

int DashSegmentTracker::threadFunction()
{
    // TODO: stop when eos
    while (!mStopLoading) {
        {
            std::unique_lock<std::mutex> locker(mSegMutex);
            mSegCondition.wait(locker, [this]() { return mNeedUpdate.load(); });
        }

        if (!mStopLoading) {
            mPlayListStatus = loadPlayList();
            mNeedUpdate = false;
        }
    }

    return 0;
}

void DashSegmentTracker::interrupt(int inter)
{
    std::unique_lock<std::recursive_mutex> locker(mMutex);
    mInterrupted = inter;

    if (mPDataSource) {
        mPDataSource->Interrupt(inter);
    }
}

bool DashSegmentTracker::isInited()
{
    return mInited;
}

void DashSegmentTracker::setCurSegNum(uint64_t num)
{
    mCurrentSegNumber = num;
}

uint64_t DashSegmentTracker::getCurSegNum()
{
    return mCurrentSegNumber;
}

bool DashSegmentTracker::isLive() const
{
    if (mPPlayList) {
        return mPPlayList->isLive();
    }
    return false;
}

std::string DashSegmentTracker::getPlayListUri()
{
    std::unique_lock<std::recursive_mutex> locker(mMutex);
    if (mRep == nullptr) {
        return "";
    }
    return Helper::combinePaths(mRep->getPlaylist()->getPlaylistUrl(), mRep->getPlaylistUrl());
}

uint64_t DashSegmentTracker::getCurSegPosition()
{
    assert(false && "not implement");
    return 0;
}

void DashSegmentTracker::setCurSegPosition(uint64_t position)
{
    assert(false && "not implement");
    return;
}

uint64_t DashSegmentTracker::getLastSegNum()
{
    assert(false && "not implement");
    return 0;
}

Representation *DashSegmentTracker::getNextRepresentation(AdaptationSet *adaptSet, Representation *rep) const
{
    if (mRep) {
        return mRep;
    }
    std::list<Representation *> reps = adaptSet->getRepresentations();
    return (reps.empty()) ? nullptr : *(reps.begin());
}

Representation *DashSegmentTracker::getCurrentRepresentation()
{
    return mRep;
}

uint64_t DashSegmentTracker::getStartSegmentNumber(Representation *rep) const
{
    if (rep->getPlaylist()->isLive()) {
        return getLiveStartSegmentNumber(rep);
    }

    const Dash::ISegmentBase *profile = rep->inheritSegmentProfile();
    return profile ? profile->getStartSegmentNumber() : 0;
}

uint64_t DashSegmentTracker::getLiveStartSegmentNumber(Representation *rep) const
{
    playList *playlist = rep->getPlaylist();

    /* Get buffering offset min <= max <= live delay */
    int64_t i_buffering = getBufferingOffset(playlist);

    Dash::SegmentList *segmentList = rep->inheritSegmentList();
    Dash::SegmentBase *segmentBase = rep->inheritSegmentBase();
    Dash::SegmentTemplate *mediaSegmentTemplate = rep->inheritSegmentTemplate();

    Dash::SegmentTimeline *timeline;
    if (mediaSegmentTemplate) {
        timeline = mediaSegmentTemplate->inheritSegmentTimeline();
    } else if (segmentList) {
        timeline = segmentList->inheritSegmentTimeline();
    } else {
        timeline = nullptr;
    }

    if (timeline) {
        uint64_t start = 0;
        const Dash::Timescale timescale = timeline->inheritTimescale();

        uint64_t safeMinElementNumber = timeline->minElementNumber();
        uint64_t safeMaxElementNumber = timeline->maxElementNumber();
        int64_t safeedgetime, safestarttime, duration;
        for (unsigned i = 0; i < SAFETY_BUFFERING_EDGE_OFFSET; i++) {
            if (safeMinElementNumber == safeMaxElementNumber) {
                break;
            }
            safeMaxElementNumber--;
        }
        bool b_ret = timeline->getScaledPlaybackTimeDurationBySegmentNumber(safeMaxElementNumber, &safeedgetime, &duration);
        if (!b_ret) {
            return 0;
        }
        safeedgetime += duration - 1;

        for (unsigned i = 0; i < SAFETY_EXPURGING_OFFSET; i++) {
            if (safeMinElementNumber + 1 >= safeMaxElementNumber) {
                break;
            }
            safeMinElementNumber++;
        }
        b_ret = timeline->getScaledPlaybackTimeDurationBySegmentNumber(safeMinElementNumber, &safestarttime, &duration);
        if (!b_ret) {
            return 0;
        }

        if (playlist->timeShiftBufferDepth > 0) {
            int64_t edgetime;
            b_ret = timeline->getScaledPlaybackTimeDurationBySegmentNumber(timeline->maxElementNumber(), &edgetime, &duration);
            if (!b_ret) {
                return 0;
            }
            edgetime += duration - 1;
            int64_t timeshiftdepth = timescale.ToScaled(playlist->timeShiftBufferDepth);
            if (safestarttime + timeshiftdepth < edgetime) {
                safestarttime = edgetime - timeshiftdepth;
                safeMinElementNumber = timeline->getElementNumberByScaledPlaybackTime(safestarttime);
            }
        }
        assert(safestarttime <= safeedgetime);

        int64_t starttime;
        if (safeedgetime - safestarttime > timescale.ToScaled(i_buffering)) {
            starttime = safeedgetime - timescale.ToScaled(i_buffering);
        } else {
            starttime = safestarttime;
        }

        start = timeline->getElementNumberByScaledPlaybackTime(starttime);
        assert(start >= timeline->minElementNumber());
        assert(start >= safeMinElementNumber);
        assert(start <= timeline->maxElementNumber());
        assert(start <= safeMaxElementNumber);

        return start;
    } else if (mediaSegmentTemplate) {
        /* Else compute, current time and timeshiftdepth based */
        uint64_t start = 0;
        int64_t scaledduration = mediaSegmentTemplate->inheritDuration();
        if (scaledduration) {
            /* Compute playback offset and effective finished segment from wall time */
            int64_t now = playlist->GetUtcTime();
            int64_t playbacktime = now - i_buffering;
            int64_t minavailtime = playlist->availabilityStartTime + rep->getPeriodStart();
            const uint64_t startnumber = mediaSegmentTemplate->inheritStartNumber();
            const Dash::Timescale timescale = mediaSegmentTemplate->inheritTimescale();
            if (!timescale) {
                return startnumber;
            }
            const int64_t duration = timescale.ToTime(scaledduration);
            if (!duration) {
                return startnumber;
            }

            /* restrict to DVR window */
            if (playlist->timeShiftBufferDepth) {
                int64_t elapsed = now - minavailtime;
                elapsed = elapsed - (elapsed % duration); /* align to last segment */
                int64_t alignednow = minavailtime + elapsed;
                if (playlist->timeShiftBufferDepth < elapsed) {
                    minavailtime = alignednow - playlist->timeShiftBufferDepth;
                }

                if (playbacktime < minavailtime) {
                    playbacktime = minavailtime;
                }
            }
            /* Get completed segment containing the time ref */
            start = mediaSegmentTemplate->getLiveTemplateNumber(playbacktime);
            if (start < startnumber) {
                assert(startnumber > start); /* blame getLiveTemplateNumber() */
                start = startnumber;
            }

            const uint64_t max_safety_offset = playbacktime - minavailtime / duration;
            const uint64_t safety_offset = std::min((uint64_t) SAFETY_BUFFERING_EDGE_OFFSET, max_safety_offset);
            if (startnumber + safety_offset <= start) {
                start -= safety_offset;
            } else {
                start = startnumber;
            }

            return start;
        }
    } else if (segmentList && !segmentList->getSegments().empty()) {
        const Dash::Timescale timescale = segmentList->inheritTimescale();
        const std::vector<Dash::DashSegment *> &list = segmentList->getSegments();
        const Dash::DashSegment *back = list.back();

        /* working around HLS discontinuities by using durations */
        int64_t totallistduration = 0;
        for (auto it = list.begin(); it != list.end(); ++it) {
            totallistduration += (*it)->duration;
        }

        /* Apply timeshift restrictions */
        int64_t availableduration;
        if (playlist->timeShiftBufferDepth) {
            availableduration = std::min(totallistduration, timescale.ToScaled(playlist->timeShiftBufferDepth));
        } else {
            availableduration = totallistduration;
        }

        uint64_t availableliststartnumber = list.front()->getSequenceNumber();
        if (totallistduration != availableduration) {
            int64_t offset = totallistduration - availableduration;
            for (auto it = list.begin(); it != list.end(); ++it) {
                availableliststartnumber = (*it)->getSequenceNumber();
                if (offset < (*it)->duration) {
                    break;
                }
                offset -= (*it)->duration;
            }
        }

        uint64_t safeedgenumber = back->getSequenceNumber() - std::min((uint64_t) list.size() - 1, (uint64_t) SAFETY_BUFFERING_EDGE_OFFSET);
        uint64_t safestartnumber = availableliststartnumber;

        for (unsigned i = 0; i < SAFETY_EXPURGING_OFFSET; i++) {
            if (safestartnumber + 1 >= safeedgenumber) {
                break;
            }
            safestartnumber++;
        }

        int64_t maxbufferizable = 0;
        int64_t safeedgeduration = 0;
        for (auto it = list.begin(); it != list.end(); ++it) {
            if ((*it)->getSequenceNumber() < safestartnumber) {
                continue;
            }
            if ((*it)->getSequenceNumber() <= safeedgenumber) {
                maxbufferizable += (*it)->duration;
            } else {
                safeedgeduration += (*it)->duration;
            }
        }

        int64_t tobuffer = std::min(maxbufferizable, timescale.ToScaled(i_buffering));
        int64_t skipduration = totallistduration - safeedgeduration - tobuffer;
        uint64_t start = safestartnumber;
        for (auto it = list.begin(); it != list.end(); ++it) {
            start = (*it)->getSequenceNumber();
            if ((*it)->duration > skipduration) {
                break;
            }
            skipduration -= (*it)->duration;
        }

        return start;
    } else if (segmentBase) {
        const std::vector<Dash::DashSegment *> &list = segmentBase->subsegments;
        if (!list.empty()) {
            return segmentBase->getSequenceNumber();
        }

        const Dash::Timescale timescale = rep->inheritTimescale();
        if (!timeline->isValid()) {
            return std::numeric_limits<uint64_t>::max();
        }
        const Dash::DashSegment *back = list.back();
        const int64_t bufferingstart = back->startTime + back->duration - timescale.ToScaled(i_buffering);

        uint64_t start = Dash::ISegmentBase::findSegmentNumberByScaledTime(list, bufferingstart);
        if (start == std::numeric_limits<uint64_t>::max()) {
            return list.front()->getSequenceNumber();
        }

        if (segmentBase->getSequenceNumber() + SAFETY_BUFFERING_EDGE_OFFSET <= start) {
            start -= SAFETY_BUFFERING_EDGE_OFFSET;
        } else {
            start = segmentBase->getSequenceNumber();
        }

        return start;
    }

    return std::numeric_limits<uint64_t>::max();
}

int64_t DashSegmentTracker::getBufferingOffset(const playList *p) const
{
    return p->isLive() ? getLiveDelay(p) : getMaxBuffering(p);
}

int64_t DashSegmentTracker::getLiveDelay(const playList *p) const
{
    int64_t delay = std::stoll(mOpts->get("RTMaxDelayTime"));
    if (p == nullptr) {
        return delay;
    }
    if (isLowLatency(p)) {
        return getMinBuffering(p);
    }
    if (p->suggestedPresentationDelay > 0) {
        delay = p->suggestedPresentationDelay;
    }
    if (p->timeShiftBufferDepth > 0) {
        delay = std::min(delay, p->timeShiftBufferDepth);
    }
    return std::max(delay, getMinBuffering(p));
}

int64_t DashSegmentTracker::getMaxBuffering(const playList *p) const
{
    if (isLowLatency(p)) {
        return getMinBuffering(p);
    }

    int64_t buffering = std::stoll(mOpts->get("maxBufferDuration"));
    if (p->isLive()) {
        buffering = std::min(buffering, getLiveDelay(p));
    }
    if (p->maxBufferTime > 0) {
        buffering = std::min(buffering, p->maxBufferTime);
    }
    return std::max(buffering, getMinBuffering(p));
}

int64_t DashSegmentTracker::getMinBuffering(const playList *p) const
{
    if (isLowLatency(p)) {
        return BUFFERING_LOWEST_LIMIT;
    }

    int64_t buffering = std::stoll(mOpts->get("highLevelBufferDuration"));
    if (p->minBufferTime > 0) {
        buffering = std::max(buffering, p->minBufferTime);
    }
    return std::max(buffering, BUFFERING_LOWEST_LIMIT);
}

bool DashSegmentTracker::isLowLatency(const playList *p) const
{
    return p->isLowLatency();
}

bool DashSegmentTracker::bufferingAvailable() const
{
    if (isLive()) {
        return getMinAheadTime() > 0;
    }
    return true;
}

int64_t DashSegmentTracker::getMinAheadTime() const
{
    if (mRep) {
        uint64_t startnumber = mCurrentSegNumber;
        if (startnumber == std::numeric_limits<uint64_t>::max()) {
            startnumber = getStartSegmentNumber(mRep);
        }
        if (startnumber != std::numeric_limits<uint64_t>::max()) {
            return mRep->getMinAheadTime(startnumber);
        }
    }
    return 0;
}

int64_t DashSegmentTracker::getLiveDelay() const
{
    if (mPPlayList == nullptr) {
        return 0;
    }
    return getLiveDelay(mPPlayList);
}