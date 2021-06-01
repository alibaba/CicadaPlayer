//
// Created by yuyuan on 2021/03/18.
//

#define LOG_TAG "DashSegmentTracker"

#include "DashSegmentTracker.h"
#include "DashSegment.h"
#include "ISegmentBase.h"
#include "MPDParser.h"
#include "SegmentBase.h"
#include "SegmentList.h"
#include "SegmentTemplate.h"
#include "SegmentTimeline.h"
#include "SidxParser.h"
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

const static int SAFETY_BUFFERING_EDGE_OFFSET = 0;
const static int SAFETY_EXPURGING_OFFSET = 2;
const static int64_t BUFFERING_LOWEST_LIMIT = 1000000 * 2;

std::atomic<int64_t> DashSegmentTracker::mLastLoadTime{10ll * 1000000};


DashSegmentTracker::DashSegmentTracker(AdaptationSet *adapt, Representation *rep, const IDataSource::SourceConfig &sourceConfig)
    : mAdapt(adapt), mRep(rep), mSourceConfig(sourceConfig)
{
    mThread = NEW_AF_THREAD(threadFunction);
    if (mRep == nullptr) {
        mRep = getNextRepresentation(mAdapt, nullptr);
    }
    if (mRep) {
        mPPlayList = mRep->getPlaylist();
    }
    if (mPPlayList && mPPlayList->minUpdatePeriod > 0) {
        mMinUpdatePeriod = mPPlayList->minUpdatePeriod;
    }
    mLastLoadTime = af_gettime_relative();
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

    if (mPDataSource) {
        mPDataSource->Interrupt(true);
        mPDataSource->Close();
        delete mPDataSource;
    }
}

bool DashSegmentTracker::parseIndex(const Dash::SidxBox &sidx, const std::string &uri, int64_t startByte, int64_t endByte)
{
    Representation::SplitPoint point;
    std::vector<Representation::SplitPoint> splitlist;
    /* sidx refers to offsets from end of sidx pos in the file + first offset */
    point.offset = sidx.first_offset + endByte + 1;
    point.time = 0;
    if (!sidx.timescale) return false;
    for (uint16_t i = 0; i < sidx.reference_count; i++) {
        splitlist.push_back(point);
        point.offset += sidx.items[i].referenced_size;
        point.duration = sidx.items[i].subsegment_duration;
        point.time += point.duration;
    }
    {
        std::unique_lock<std::recursive_mutex> locker(mMutex);
        if (mRep == nullptr) {
            return false;
        }
        Dash::DashSegment *indexSeg = mRep->getIndexSegment();
        std::string currUri = indexSeg->getUrlSegment().toString(0, mRep);
        int64_t currStartByte = indexSeg->startByte;
        if (currStartByte < 0) {
            currStartByte = 0;
        }
        int64_t currEndByte = indexSeg->endByte;
        if (uri == currUri && startByte == currStartByte && endByte == currEndByte) {
            mRep->replaceAttribute(new Dash::TimescaleAttr(Dash::Timescale(sidx.timescale)));
            mRep->SplitUsingIndex(splitlist);
        }
    }
    return true;
}

Dash::DashSegment *DashSegmentTracker::getStartSegment()
{
    Dash::DashSegment *segment = nullptr;
    {
        std::unique_lock<std::recursive_mutex> locker(mMutex);
        uint64_t startNumber = getStartSegmentNumber(mRep);
        if (mCurrentSegNumber < startNumber || mCurrentSegNumber == std::numeric_limits<uint64_t>::max()) {
            mCurrentSegNumber = startNumber;
        }

        bool b_gap = false;
        segment = mRep->getNextMediaSegment(mCurrentSegNumber, &mCurrentSegNumber, &b_gap);
        if (segment == nullptr) {
            return nullptr;
        }
        if (b_gap) {
            --mCurrentSegNumber;
        }
        if (segment->startTime == 0) {
            segment->fixedStartTime = mRep->getMediaSegmentStartTime(mCurrentSegNumber);
        } else {
            const Dash::Timescale timescale = mRep->inheritTimescale();
            segment->fixedStartTime = timescale.ToTime(segment->startTime);
        }
    }
    return segment;
}

Dash::DashSegment *DashSegmentTracker::getNextSegment()
{
    std::unique_lock<std::recursive_mutex> locker(mMutex);

    if (mCurrentSegNumber == std::numeric_limits<uint64_t>::max()) {
        return getStartSegment();
    }

    if (mRep == nullptr) {
        return nullptr;
    }
    Dash::DashSegment *segment = nullptr;
    bool b_gap = false;
    ++mCurrentSegNumber;
    segment = mRep->getNextMediaSegment(mCurrentSegNumber, &mCurrentSegNumber, &b_gap);
    if (segment == nullptr) {
        return nullptr;
    }
    if (b_gap) {
        --mCurrentSegNumber;
    }
    if (segment->startTime == 0) {
        segment->fixedStartTime = mRep->getMediaSegmentStartTime(mCurrentSegNumber);
    } else {
        const Dash::Timescale timescale = mRep->inheritTimescale();
        segment->fixedStartTime = timescale.ToTime(segment->startTime);
    }
    return segment;
}

Dash::DashSegment *DashSegmentTracker::getInitSegment()
{
    std::unique_lock<std::recursive_mutex> locker(mMutex);
    auto rep = getNextRepresentation(mAdapt, nullptr);
    if (rep) {
        return rep->getInitSegment();
    }
    return nullptr;
}

Dash::DashSegment *DashSegmentTracker::getIndexSegment()
{
    std::unique_lock<std::recursive_mutex> locker(mMutex);
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
    int64_t remainDuration = getMinAheadTime();
    const int64_t segDuration = getSegmentDuration();
    if (segDuration <= 0) {
        return count;
    }
    count = remainDuration / segDuration;
    //AF_LOGD("[dash] GetRemainSegmentCount = %d", count);
    return count;
}

int DashSegmentTracker::loadPlayList()
{
    int ret = 0;

    string uri;

    if (mRep == nullptr || mPPlayList == nullptr) {
        return -EINVAL;
    }

    {
        std::unique_lock<std::recursive_mutex> locker(mMutex);
        uri = Helper::combinePaths(mRep->getPlaylist()->getPlaylistUrl(), mRep->getPlaylistUrl());
    }

    AF_LOGD("[dash] DashSegmentTracker::loadPlayList, uri is [%s]\n", uri.c_str());

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

    AF_LOGD("ret is %d\n", ret);

    if (ret < 0) {
        AF_LOGE("open url error %s\n", framework_err2_string(ret));
        return ret;
    }

    auto *parser = new Dash::MPDParser(uri.c_str());
    parser->setDataSourceIO(new dataSourceIO(mPDataSource));
    playList *pPlayList = parser->parse(uri);

    if (pPlayList) {
        std::unique_lock<std::recursive_mutex> locker(mMutex);
        mPPlayList->updateWith(pPlayList);
        delete pPlayList;
    }

    delete parser;
    return 0;
}

int DashSegmentTracker::init()
{
    int ret = 0;

    if (!mInited) {
        if (isLive()) {
            mThread->start();
        }

        std::string indexUri;
        int64_t startByte = 0;
        int64_t endByte = 0;
        {
            std::unique_lock<std::recursive_mutex> locker(mMutex);
            if (mRep == nullptr) {
                return ret;
            }
            if (mRep->needsIndex()) {
                Dash::DashSegment *indexSeg = mRep->getIndexSegment();
                indexUri = indexSeg->getUrlSegment().toString(0, mRep);
                startByte = indexSeg->startByte;
                if (startByte < 0) {
                    startByte = 0;
                }
                endByte = indexSeg->endByte;
            }
        }

        if (!indexUri.empty() && (endByte <= 0 || endByte >= startByte)) {
            IDataSource *dataSource = dataSourcePrototype::create(indexUri, nullptr);
            dataSource->Open(0);
            int64_t size = 0;
            if (endByte <= 0) {
                size = dataSource->Seek(0, SEEK_SIZE);
            } else {
                size = endByte - startByte + 1;
            }
            if (startByte > 0) {
                dataSource->Seek(startByte, SEEK_SET);
            }
            auto *buffer = static_cast<uint8_t *>(malloc(size));
            int len = 0;
            while (len < size) {
                int ret = dataSource->Read(buffer + len, size - len);
                if (ret > 0) {
                    len += ret;
                } else {
                    break;
                }
            }
            if (len > 0) {
                Dash::SidxParser sidxParser;
                sidxParser.ParseSidx(buffer, size);
                parseIndex(sidxParser.GetSidxBox(), indexUri, startByte, endByte);
            }
            free(buffer);
            delete dataSource;
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
    std::unique_lock<std::recursive_mutex> locker(mMutex);
    if (mRep == nullptr) {
        return -1;
    }
    return mRep->getStreamInfo(width, height, bandwidth, language);
}

std::string DashSegmentTracker::getDescriptionInfo()
{
    std::unique_lock<std::recursive_mutex> locker(mMutex);
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
        int64_t reloadInterval = mMinUpdatePeriod;
        int64_t time = af_gettime_relative();
        if (time - mLastLoadTime > reloadInterval) {
            mLastLoadTime = time;
            {
                std::unique_lock<std::mutex> locker(mSegMutex);
                mNeedUpdate = true;
                mSegCondition.notify_all();
            }
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

bool DashSegmentTracker::isInited() const
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
    return getCurSegNum();
}

void DashSegmentTracker::setCurSegPosition(uint64_t position)
{
    setCurSegNum(position);
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
        for (int i = 0; i < SAFETY_BUFFERING_EDGE_OFFSET; i++) {
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
        /* Compute playback offset and effective finished segment from wall time */
        int64_t now = af_get_utc_time();
        int64_t playbacktime = now - i_buffering;
        int64_t minavailtime = playlist->availabilityStartTime + rep->getPeriodStart();
        const uint64_t startnumber = mediaSegmentTemplate->inheritStartNumber();
        const int64_t duration = getSegmentDuration();
        if (duration <= 0) {
            return startnumber;
        }
        if (i_buffering >= duration * 2) {
            playbacktime += duration;
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
        if (!timeline || !timeline->isValid()) {
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
    int64_t delay = std::strtoll(mOpts->get("RTMaxDelayTime").c_str(), nullptr, 0);
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

    int64_t buffering = std::strtoll(mOpts->get("maxBufferDuration").c_str(), nullptr, 0);
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

    int64_t buffering = std::strtoll(mOpts->get("highLevelBufferDuration").c_str(), nullptr, 0);
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

int64_t DashSegmentTracker::getDurationToStartStream() const
{
    if (mPPlayList == nullptr || mRep == nullptr) {
        return -1;
    }
    int64_t minavailtime = mPPlayList->availabilityStartTime + mRep->getPeriodStart();
    int64_t duration = af_get_utc_time() - (minavailtime + getLiveDelay());
    return duration;
}

int64_t DashSegmentTracker::getSegmentDuration() const
{
    auto rep = getNextRepresentation(mAdapt, nullptr);
    if (rep == nullptr) {
        return 0;
    }
    const Dash::ISegmentBase *profile = rep->inheritSegmentProfile();
    if (profile == nullptr) {
        return 0;
    }
    int64_t scaledSegDuration = profile->inheritDuration();
    const Dash::Timescale timescale = profile->inheritTimescale();
    const int64_t segDuration = timescale.ToTime(scaledSegDuration);
    return segDuration;
}

int64_t DashSegmentTracker::getStreamStartTime() const
{
    if (mPPlayList == nullptr || mRep == nullptr) {
        return -1;
    }
    return mPPlayList->availabilityStartTime + mRep->getPeriodStart();
}
