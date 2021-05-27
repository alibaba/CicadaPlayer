//
// Created by yuyuan on 2021/03/15.
//

#include "DashSegment.h"

using namespace Cicada::Dash;

DashSegment::DashSegment(IDashUrl *parent) : IDashUrl(parent)
{}

DashSegment::DashSegment(IDashUrl *parent, int64_t start, int64_t end) : IDashUrl(parent)
{
    setByteRange(start, end);
}

DashSegment::~DashSegment()
{}

void DashSegment::setByteRange(int64_t start, int64_t end)
{
    startByte = start;
    endByte = end;
}

void DashSegment::setSourceUrl(const std::string &url)
{
    if (!url.empty()) {
        this->sourceUrl = DashUrl(url);
    }
}

DashUrl DashSegment::getUrlSegment() const
{
    if (sourceUrl.hasScheme()) {
        return sourceUrl;
    } else {
        DashUrl ret = getParentUrlSegment();
        if (!sourceUrl.empty()) ret.append(sourceUrl);
        return ret;
    }
}

size_t DashSegment::getOffset() const
{
    return startByte;
}

bool DashSegment::contains(size_t byte) const
{
    if (startByte == endByte) {
        return false;
    }
    return (byte >= startByte && (endByte <= 0 || byte <= endByte));
}

void DashSegment::addSubSegment(DashSegment *subsegment)
{
    if (!subsegments.empty()) {
        /* Use our own sequence number, and since it it now
           uneffective, also for next subsegments numbering */
        subsegment->setSequenceNumber(subsegments.size());
    }
    subsegments.push_back(subsegment);
}

void DashSegment::setSequenceNumber(uint64_t seq)
{
    sequence = seq;
}

uint64_t DashSegment::getSequenceNumber() const
{
    return sequence;
}

int DashSegment::compare(DashSegment *other) const
{
    if (duration) {
        if (startTime > other->startTime)
            return 1;
        else if (startTime < other->startTime)
            return -1;
    }

    if (startByte > other->startByte)
        return 1;
    else if (startByte < other->startByte)
        return -1;

    if (endByte > other->endByte)
        return 1;
    else if (endByte < other->endByte)
        return -1;

    return 0;
}