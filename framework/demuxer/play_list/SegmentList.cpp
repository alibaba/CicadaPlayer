//
// Created by moqi on 2018/4/26.
//
#define LOG_TAG "SegmentList"

#include "SegmentList.h"
#include <utils/frame_work_log.h>

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
        if (!seg->mUri.empty()) {
            mNextStartTime = seg->startTime + seg->duration;
        }
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
            if ((*i)->sequence < mLastSeqNum) {
                (*i) = nullptr;
            } else if ((*i)->sequence == mLastSeqNum) {
                if ((*i)->mSegType == SEG_LHLS) {
                    updateLastLHLSSegment((*i));
                } else {
                    (*i) = nullptr;
                }
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

    void SegmentList::updateLastLHLSSegment(const std::shared_ptr<segment> &seg)
    {
        std::lock_guard<std::mutex> uMutex(segmetsMuxtex);

        if (segments.size() > 0) {
            std::shared_ptr<segment> old_seg = segments.back();
            if (old_seg != nullptr && old_seg->sequence == mLastSeqNum && old_seg->mUri.empty()) {
                if (old_seg != nullptr && seg != nullptr) {
                    old_seg->updateParts(seg->getSegmentParts());
                    if (!seg->mUri.empty()) {
                        old_seg->duration = seg->duration;
                        mNextStartTime += old_seg->duration;
                        old_seg->setSourceUrl(seg->mUri);
                    }
                }
            }
        }
    }

    bool SegmentList::hasLHLSSegments()
    {
        bool ret = false;

        for (auto seg : segments) {
            if (seg->mSegType == SEG_LHLS) {
                ret = true;
                break;
            }
        }

        return ret;
    }

    int64_t SegmentList::getTargetDuration()
    {
        if (mRep == nullptr) {
            return INT64_MIN;
        }
        return mRep->targetDuration;
    }
}// namespace Cicada
