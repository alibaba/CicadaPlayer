//
// Created by moqi on 2018/4/25.
//
#include "segment.h"
#include "../../utils/frame_work_log.h"

namespace Cicada {
    const int segment::SEQUENCE_INVALID = 0;
    const int segment::SEQUENCE_FIRST = 1;

    segment::segment(uint64_t seq)
    {
        sequence = seq;
    }

    segment::~segment()
    {
        //   AF_TRACE;
    }

    void segment::setSequenceNumber(uint64_t seq)
    {
        sequence = /*SEQUENCE_FIRST +*/ seq;
    }

    uint64_t segment::getSequenceNumber()
    {
        return sequence;
    }

    void segment::setSourceUrl(const std::string &value)
    {
        mUri = value;
    }

    void segment::print()
    {
        AF_LOGD("segment %llu,"
                " %s duration %lld startTime is %llu\n", sequence, mUri.c_str(), duration, startTime);
    }

    void segment::setEncryption(SegmentEncryption enc)
    {
        encryption = enc;
    }

    void segment::setByteRange(int64_t start, int64_t end)
    {
        rangeStart = start;
        rangeEnd = end;
    }
}