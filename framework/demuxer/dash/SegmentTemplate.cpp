//
// Created by yuyuan on 2021/03/09.
//

#include "SegmentTemplate.h"
#include "SegmentInformation.h"
#include "SegmentTimeline.h"
#include "demuxer/play_list/playList.h"
#include "utils/timer.h"
#include <algorithm>
#include <time.h>

using namespace Cicada;
using namespace Cicada::Dash;

SegmentTemplateSegment::SegmentTemplateSegment(IDashUrl *parent) : DashSegment(parent)
{
    templated = true;
    templ = nullptr;
}

SegmentTemplateSegment::~SegmentTemplateSegment()
{}

void SegmentTemplateSegment::setSourceUrl(const std::string &url)
{
    sourceUrl = DashUrl(DashUrl::Component(url, templ));
}

void SegmentTemplateSegment::setParentTemplate(SegmentTemplate *templ_)
{
    templ = templ_;
}

SegmentTemplate::SegmentTemplate(SegmentTemplateSegment *seg, SegmentInformation *parent)
    : ISegmentBase(parent, AbstractAttr::Type::SegmentTemplate)
{
    setInitSegment(nullptr);
    parentSegmentInformation = parent;
    virtualsegment = seg;
    virtualsegment->setParent(parentSegmentInformation);
    virtualsegment->setParentTemplate(this);
}

SegmentTemplate::~SegmentTemplate()
{
    delete virtualsegment;
}

void SegmentTemplate::setSourceUrl(const std::string &url)
{
    virtualsegment->setSourceUrl(url);
}

void SegmentTemplate::pruneByPlaybackTime(int64_t time)
{
    AbstractAttr *p = getAttribute(Type::Timeline);
    if (p) {
        return static_cast<SegmentTimeline *>(p)->pruneByPlaybackTime(time);
    }
}

size_t SegmentTemplate::pruneBySequenceNumber(uint64_t number)
{
    AbstractAttr *p = getAttribute(Type::Timeline);
    if (p) {
        return static_cast<SegmentTimeline *>(p)->pruneBySequenceNumber(number);
    }
    return 0;
}

uint64_t SegmentTemplate::getLiveTemplateNumber(int64_t playbacktime, bool abs) const
{
    uint64_t number = inheritStartNumber();
    /* live streams / templated */
    const int64_t dur = inheritDuration();
    if (dur) {
        /* compute, based on current time */
        /* N = (T - AST - PS - D)/D + sSN */
        const Timescale timescale = inheritTimescale();
        if (abs) {
            int64_t streamstart = parentSegmentInformation->getPlayList()->availabilityStartTime;
            streamstart += parentSegmentInformation->getPeriodStart();
            playbacktime -= streamstart;
        }
        if (playbacktime < 0) {
            playbacktime = 0;
        }
        int64_t elapsed = timescale.ToScaled(playbacktime) - dur;
        if (elapsed > 0) {
            number += elapsed / dur;
        }
    }

    return number;
}

int64_t SegmentTemplate::getMinAheadTime(uint64_t number) const
{
    SegmentTimeline *timeline = inheritSegmentTimeline();
    if (timeline) {
        const Timescale timescale = timeline->inheritTimescale();
        return timescale.ToTime(timeline->getMinAheadScaledTime(number));
    } else {
        const Timescale timescale = inheritTimescale();
        uint64_t current = getLiveTemplateNumber(af_get_utc_time());
        int64_t i_length = (current - number) * inheritDuration();
        return timescale.ToTime(i_length);
    }
}

DashSegment *SegmentTemplate::getMediaSegment(uint64_t number) const
{
    const SegmentTimeline *tl = inheritSegmentTimeline();
    if (tl == nullptr || (tl->maxElementNumber() >= number && tl->minElementNumber() <= number)) {
        return virtualsegment;
    }
    return nullptr;
}

DashSegment *SegmentTemplate::getInitSegment() const
{
    return ISegmentBase::getInitSegment();
}

DashSegment *SegmentTemplate::getNextMediaSegment(uint64_t i_pos, uint64_t *pi_newpos, bool *pb_gap) const
{
    *pb_gap = false;
    *pi_newpos = i_pos;
    /* Check if we don't exceed timeline */
    const SegmentTimeline *timeline = inheritSegmentTimeline();
    if (timeline) {
        *pi_newpos = std::max(timeline->minElementNumber(), i_pos);
        if (timeline->maxElementNumber() < i_pos) {
            return nullptr;
        }
    } else {
        /* check template upper bound */
        const playList *playlist = parentSegmentInformation->getPlayList();
        const Timescale timescale = inheritTimescale();
        const int64_t segmentduration = inheritDuration();
        int64_t totalduration = parentSegmentInformation->getPeriodDuration();
        if (totalduration == 0) {
            totalduration = playlist->getDuration();
        }
        if (totalduration && segmentduration) {
            uint64_t endnum = inheritStartNumber() + (timescale.ToScaled(totalduration) + segmentduration - 1) / segmentduration;
            if (i_pos >= endnum - 1) {
                *pi_newpos = i_pos;
                return nullptr;
            }
        }
        *pi_newpos = i_pos;
        /* start number */
        *pi_newpos = std::max(inheritStartNumber(), i_pos);
    }
    return virtualsegment;
}

uint64_t SegmentTemplate::getStartSegmentNumber() const
{
    const SegmentTimeline *timeline = inheritSegmentTimeline();
    return timeline ? timeline->minElementNumber() : inheritStartNumber();
}

bool SegmentTemplate::getSegmentNumberByTime(int64_t time, uint64_t *ret) const
{
    const SegmentTimeline *timeline = inheritSegmentTimeline();
    if (timeline) {
        const Timescale timescale = timeline->inheritTimescale();
        int64_t st = timescale.ToScaled(time);
        *ret = timeline->getElementNumberByScaledPlaybackTime(st);
        return true;
    }

    const int64_t duration = inheritDuration();
    if (duration && mParent) {
        playList *playlist = mParent->getPlayList();
        if (playlist->isLive()) {
            int64_t now = af_get_utc_time();
            if (playlist->availabilityStartTime) {
                if (time >= playlist->availabilityStartTime && time < now) {
                    *ret = getLiveTemplateNumber(time, true);
                } else if (now - playlist->availabilityStartTime > time) {
                    *ret = getLiveTemplateNumber(time, false);
                }
            } else {
                return false;
            }
        } else {
            const Timescale timescale = inheritTimescale();
            *ret = inheritStartNumber();
            *ret += timescale.ToScaled(time) / duration;
        }
        return true;
    }

    return false;
}


bool SegmentTemplate::getPlaybackTimeDurationBySegmentNumber(uint64_t number, int64_t *time, int64_t *duration) const
{
    if (number == std::numeric_limits<uint64_t>::max()) {
        return false;
    }

    Timescale timescale;
    int64_t stime, sduration;

    const SegmentTimeline *timeline = inheritSegmentTimeline();
    if (timeline) {
        timescale = timeline->inheritTimescale();
        if (!timeline->getScaledPlaybackTimeDurationBySegmentNumber(number, &stime, &sduration)) {
            return false;
        }
    } else {
        timescale = inheritTimescale();
        uint64_t startNumber = inheritStartNumber();
        if (number < startNumber) {
            return false;
        }
        sduration = inheritDuration();
        stime = (number - startNumber) * sduration;
    }

    *time = timescale.ToTime(stime);
    *duration = timescale.ToTime(sduration);
    return true;
}

SegmentTemplateInit::SegmentTemplateInit(SegmentTemplate *templ_, IDashUrl *parent) : DashSegment(parent)
{
    templ = templ_;
}

SegmentTemplateInit::~SegmentTemplateInit()
{}

void SegmentTemplateInit::setSourceUrl(const std::string &url)
{
    sourceUrl = DashUrl(DashUrl::Component(url, templ));
}