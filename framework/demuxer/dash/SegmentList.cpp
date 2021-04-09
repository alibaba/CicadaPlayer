//
// Created by yuyuan on 2021/03/09.
//

#include "SegmentList.h"
#include "DashSegment.h"
#include "SegmentInformation.h"
#include "SegmentTimeline.h"
#include <limits>

using namespace Cicada::Dash;

SegmentList::SegmentList(SegmentInformation *parent) : ISegmentBase(parent, AttrsNode::Type::SegmentList)
{
    totalLength = 0;
}

SegmentList::~SegmentList()
{
    std::vector<DashSegment *>::iterator it;
    for (it = segments.begin(); it != segments.end(); ++it) {
        delete (*it);
    }
}

const std::vector<DashSegment *> &SegmentList::getSegments() const
{
    return segments;
}

DashSegment *SegmentList::getMediaSegment(uint64_t number) const
{
    const SegmentTimeline *timeline = inheritSegmentTimeline();
    if (timeline) {
        uint64_t listindex = timeline->getElementIndexBySequence(number);
        if (listindex >= segments.size()) {
            return nullptr;
        }
        return segments.at(listindex);
    }

    std::vector<DashSegment *>::const_iterator it = segments.begin();
    for (it = segments.begin(); it != segments.end(); ++it) {
        DashSegment *seg = *it;
        if (seg->getSequenceNumber() == number) {
            return seg;
        } else if (seg->getSequenceNumber() > number) {
            break;
        }
    }
    return nullptr;
}

void SegmentList::addSegment(DashSegment *seg)
{
    seg->setParent(ISegmentBase::mParent);
    segments.push_back(seg);
    totalLength += seg->duration;
}

void SegmentList::updateWith(ISegmentBase *updated_, bool b_restamp)
{
    ISegmentBase::updateWith(updated_);

    SegmentList *updated = dynamic_cast<SegmentList *>(updated_);
    if (!updated || updated->segments.empty()) {
        return;
    }

    const DashSegment *lastSegment = (segments.empty()) ? nullptr : segments.back();
    const DashSegment *prevSegment = lastSegment;

    uint64_t firstnumber = updated->segments.front()->getSequenceNumber();

    std::vector<DashSegment *>::iterator it;
    for (it = updated->segments.begin(); it != updated->segments.end(); ++it) {
        DashSegment *cur = *it;
        if (!lastSegment || lastSegment->compare(cur) < 0) {
            if (b_restamp && prevSegment) {
                int64_t starttime = prevSegment->startTime + prevSegment->duration;
                if (starttime != cur->startTime && !cur->discontinuity) {
                    cur->startTime = starttime;
                }

                prevSegment = cur;
            }
            addSegment(cur);
        } else
            delete cur;
    }
    updated->segments.clear();

    pruneBySegmentNumber(firstnumber);
}

void SegmentList::pruneByPlaybackTime(int64_t time)
{
    const Timescale timescale = inheritTimescale();
    uint64_t num = findSegmentNumberByScaledTime(segments, timescale.ToScaled(time));
    if (num != std::numeric_limits<uint64_t>::max()) {
        pruneBySegmentNumber(num);
    }
}

void SegmentList::pruneBySegmentNumber(uint64_t tobelownum)
{
    std::vector<DashSegment *>::iterator it = segments.begin();
    while (it != segments.end()) {
        DashSegment *seg = *it;

        if (seg->getSequenceNumber() >= tobelownum) {
            break;
        }

        totalLength -= (*it)->duration;
        delete *it;
        it = segments.erase(it);
    }
}

bool SegmentList::getPlaybackTimeDurationBySegmentNumber(uint64_t number, int64_t *time, int64_t *dur) const
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
        *time = 0;
        *dur = 0;
        timescale = inheritTimescale();

        if (segments.empty()) {
            return false;
        }

        const DashSegment *first = segments.front();
        if (first->getSequenceNumber() > number) {
            return false;
        }

        bool found = false;
        stime = first->startTime;
        sduration = 0;
        std::vector<DashSegment *>::const_iterator it;
        for (it = segments.begin(); it != segments.end(); ++it) {
            const DashSegment *seg = *it;

            if (seg->duration) {
                sduration = seg->duration;
            } else {
                sduration = inheritDuration();
            }

            /* Assuming there won't be any discontinuity in sequence */
            if (seg->getSequenceNumber() == number) {
                found = true;
                break;
            }

            stime += sduration;
        }

        if (!found) {
            return false;
        }
    }

    *time = timescale.ToTime(stime);
    *dur = timescale.ToTime(sduration);
    return true;
}

int64_t SegmentList::getTotalLength() const
{
    const SegmentTimeline *timeline = inheritSegmentTimeline();
    if (timeline) {
        return timeline->getTotalLength();
    }
    return totalLength;
}

int64_t SegmentList::getMinAheadTime(uint64_t curnum) const
{
    const SegmentTimeline *timeline = inheritSegmentTimeline();
    if (timeline) {
        const Timescale timescale = timeline->inheritTimescale();
        return timescale.ToTime(timeline->getMinAheadScaledTime(curnum));
    }

    int64_t minTime = 0;
    const Timescale timescale = inheritTimescale();
    std::vector<DashSegment *>::const_iterator it;
    for (it = segments.begin(); it != segments.end(); ++it) {
        const DashSegment *seg = *it;
        if (seg->getSequenceNumber() > curnum) {
            minTime += timescale.ToTime(seg->duration);
        }
    }
    return minTime;
}

DashSegment *SegmentList::getNextMediaSegment(uint64_t i_pos, uint64_t *pi_newpos, bool *pb_gap) const
{
    *pb_gap = false;
    *pi_newpos = i_pos;

    const SegmentTimeline *timeline = inheritSegmentTimeline();
    if (timeline) {
        uint64_t listindex = timeline->getElementIndexBySequence(i_pos);
        if (listindex >= segments.size()) {
            return nullptr;
        }
        return segments.at(listindex);
    }

    std::vector<DashSegment *>::const_iterator it;
    for (it = segments.begin(); it != segments.end(); ++it) {
        DashSegment *seg = *it;
        if (seg->getSequenceNumber() >= i_pos) {
            *pi_newpos = seg->getSequenceNumber();
            *pb_gap = (*pi_newpos != i_pos);
            return seg;
        }
    }
    return nullptr;
}

uint64_t SegmentList::getStartSegmentNumber() const
{
    const SegmentTimeline *timeline = inheritSegmentTimeline();
    if (timeline) {
        return timeline->minElementNumber();
    }
    return !segments.empty() ? segments.front()->getSequenceNumber() : inheritStartNumber();
}

bool SegmentList::getSegmentNumberByTime(int64_t time, uint64_t *ret) const
{
    const SegmentTimeline *timeline = inheritSegmentTimeline();
    if (timeline) {
        const Timescale timescale = timeline->inheritTimescale();
        int64_t st = timescale.ToScaled(time);
        *ret = timeline->getElementNumberByScaledPlaybackTime(st);
        return true;
    }

    const Timescale timescale = inheritTimescale();
    if (!timescale.isValid()) {
        return false;
    }
    int64_t st = timescale.ToScaled(time);
    *ret = ISegmentBase::findSegmentNumberByScaledTime(segments, st);
    return *ret != std::numeric_limits<uint64_t>::max();
}
