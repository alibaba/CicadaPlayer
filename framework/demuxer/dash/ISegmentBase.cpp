//
// Created by yuyuan on 2021/03/15.
//

#include "ISegmentBase.h"
#include "DashSegment.h"
#include "SegmentTimeline.h"
#include <limits>

using namespace Cicada::Dash;

ISegmentBase::ISegmentBase(SegmentInformation *parent, Type type) : AttrsNode(type)
{
    mParent = parent;
}

ISegmentBase::~ISegmentBase()
{
    delete mInitSegment;
}

DashSegment *ISegmentBase::getInitSegment() const
{
    return mInitSegment;
}

DashSegment *ISegmentBase::getIndexSegment() const
{
    return mIndexSegment;
}

void ISegmentBase::setInitSegment(DashSegment *segment)
{
    if (segment) {
        segment->isInitSegment = true;
    }
    mInitSegment = segment;
}

void ISegmentBase::setIndexSegment(DashSegment *segment)
{
    mIndexSegment = segment;
}

void ISegmentBase::updateWith(ISegmentBase *updated, bool restamp)
{
    SegmentTimeline *local = static_cast<SegmentTimeline *>(getAttribute(Type::Timeline));
    SegmentTimeline *other = static_cast<SegmentTimeline *>(updated->getAttribute(Type::Timeline));
    if (local && other) local->updateWith(*other);
}

DashSegment *ISegmentBase::findSegmentByScaledTime(const std::vector<DashSegment *> &segments, int64_t time)
{
    if (segments.empty() || (segments.size() > 1 && segments[1]->startTime == 0)) {
        return nullptr;
    }

    DashSegment *ret = nullptr;
    std::vector<DashSegment *>::const_iterator it = segments.begin();
    while (it != segments.end()) {
        DashSegment *seg = *it;
        if (seg->startTime > time) {
            if (it == segments.begin()) {
                return nullptr;
            } else {
                break;
            }
        }

        ret = seg;
        it++;
    }

    return ret;
}

uint64_t ISegmentBase::findSegmentNumberByScaledTime(const std::vector<DashSegment *> &segments, int64_t time)
{
    DashSegment *s = findSegmentByScaledTime(segments, time);
    if (!s) {
        return std::numeric_limits<uint64_t>::max();
    }
    return s->getSequenceNumber();
}