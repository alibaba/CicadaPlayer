//
// Created by moqi on 2018/4/26.
//
#define LOG_TAG "SegmentList"

#include <utils/frame_work_log.h>
#include "SegmentList.h"

namespace Cicada {

    SegmentList::SegmentList(Representation *rep)
    {
        mRep = rep;
    }

    SegmentList::~SegmentList()
    {
        segments.clear();
    }

    list<std::shared_ptr<segment>> &SegmentList::getSegments()
    {
        return segments;
    }

    int SegmentList::getRemainSegmentAfterNumber(uint64_t number)
    {
        int i = 0;
        std::lock_guard<std::mutex> uMutex(segmetsMuxtex);

        for (auto &segment : segments) {
            if (segment->sequence > number) {
                ++i;
            }
        }

        return i;
    }

    shared_ptr<segment> SegmentList::getSegmentByNumber(uint64_t number)
    {
        std::lock_guard<std::mutex> uMutex(segmetsMuxtex);

        for (auto &segment : segments) {
            if (segment->getSequenceNumber() >= number) {
                return segment;
            }
        }

        return nullptr;
    }

    void SegmentList::addSegment(const shared_ptr<segment> &seg)
    {
        if (mFirstSeqNum < 0) {
            mFirstSeqNum = seg->sequence;
        }

        std::lock_guard<std::mutex> uMutex(segmetsMuxtex);

        if (seg->startTime == UINT64_MAX) {
            seg->startTime = mNextStartTime;
        }

        mNextStartTime = seg->startTime + seg->duration;
        mLastSeqNum = seg->sequence;
        segments.push_back(seg);
    }

    void SegmentList::print()
    {
        AF_LOGD("%d segments\n", segments.size());
        AF_LOGD("base url is %s\n", mRep->getBaseUrl().c_str());
        std::lock_guard<std::mutex> uMutex(segmetsMuxtex);

        for (auto &segment : segments) {
            segment->print();
        }
    }

    bool SegmentList::getSegmentNumberByTime(uint64_t &time, uint64_t &num)
    {
        AF_LOGI("time is %llu", time);
        uint64_t duration = 0;
        std::lock_guard<std::mutex> uMutex(segmetsMuxtex);

        for (auto &i : segments) {
            duration += i->duration;

            if (duration > time) {
                num = i->sequence;
                time = duration - i->duration;
                return true;
            }
        }

        AF_LOGE("num not found");
        return false;
    }

    int SegmentList::merge(SegmentList *pSList)
    {
        int seqNum = static_cast<int>(mLastSeqNum);
        auto &sList = pSList->getSegments();
        int size = sList.size();

        for (auto i = sList.begin(); i != sList.end();) {
            if ((*i)->sequence <= mLastSeqNum) {
                (*i) = nullptr;
            } else {
                AF_LOGI("xxxxxx add a new seg %llu", (*i)->sequence);
                (*i)->startTime = UINT64_MAX;
                addSegment(*i);
            }

            sList.erase(i++);
        }

        while (segments.size() > size) {
            segments.pop_front();
        }

        if (!segments.empty()) {
            mFirstSeqNum = segments.front()->sequence;
        }
        delete pSList;
        return 0;
    }

    uint64_t SegmentList::getFirstSeqNum() const
    {
        return static_cast<uint64_t>(mFirstSeqNum);
    }
    uint64_t SegmentList::getLastSeqNum() const
    {
        return static_cast<uint64_t>(mLastSeqNum);
    }
}
