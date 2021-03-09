//
// Created by yuyuan on 2021/03/09.
//

#include "SegmentBase.h"
#include "SegmentInformation.h"
#include <algorithm>

using namespace Cicada::Dash;

SegmentBase::SegmentBase(SegmentInformation *parent) : DashSegment(parent), ISegmentBase(parent, AttrsNode::Type::SegmentBase)
{
    this->mParent = parent;
}

SegmentBase::~SegmentBase()
{}

int64_t SegmentBase::getMinAheadTime(uint64_t curnum) const
{
    if (subsegments.size() == 0 || curnum >= subsegments.size() - 1) {
        return 0;
    }

    const Timescale timescale = inheritTimescale();
    if (!timescale.isValid()) {
        return 0;
    }

    int64_t minTime = 0;
    std::for_each(subsegments.cbegin() + curnum + 1, subsegments.cend(),
                  [&minTime, timescale](const DashSegment *seg) { minTime += seg->duration; });

    return timescale.ToTime(minTime);
}

DashSegment *SegmentBase::getMediaSegment(uint64_t pos) const
{
    return (pos < subsegments.size()) ? subsegments.at(pos) : nullptr;
}

DashSegment *SegmentBase::getNextMediaSegment(uint64_t i_pos, uint64_t *pi_newpos, bool *pb_gap) const
{
    *pb_gap = false;
    *pi_newpos = i_pos;
    return getMediaSegment(i_pos);
}

uint64_t SegmentBase::getStartSegmentNumber() const
{
    return 0;
}

bool SegmentBase::getSegmentNumberByTime(int64_t time, uint64_t *ret) const
{
    const Timescale timescale = inheritTimescale();
    if (!timescale.isValid()) {
        return false;
    }
    int64_t st = timescale.ToScaled(time);
    *ret = ISegmentBase::findSegmentNumberByScaledTime(subsegments, st);
    return *ret != std::numeric_limits<uint64_t>::max();
}

bool SegmentBase::getPlaybackTimeDurationBySegmentNumber(uint64_t number, int64_t *time, int64_t *dur) const
{
    const Timescale timescale = inheritTimescale();
    const DashSegment *segment = getMediaSegment(number);
    if (segment) {
        *time = timescale.ToTime(segment->startTime);
        *dur = timescale.ToTime(segment->duration);
        return true;
    }
    return false;
}