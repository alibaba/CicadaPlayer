//
// Created by moqi on 2018/4/26.
//

#ifndef FRAMEWORK_SEGMENTLIST_H
#define FRAMEWORK_SEGMENTLIST_H

#include <list>
#include "segment.h"
#include "Representation.h"
#include <mutex>
#include <memory>

namespace Cicada{
    class Representation;

    class SegmentList {
    public:
        SegmentList(Representation *rep);

        ~SegmentList();

        std::list<std::shared_ptr<segment>> &getSegments();

        std::shared_ptr<segment> getSegmentByNumber(uint64_t number);

        bool getSegmentNumberByTime(uint64_t &time, uint64_t &num);

        int getRemainSegmentAfterNumber(uint64_t number);

        void addSegment(const std::shared_ptr<segment> &seg);

        void addInitSegment(const std::shared_ptr<segment> &seg)
        {
            initSegment.push_back(seg);
        }

        int merge(SegmentList *pSList);

        void print();

        uint64_t getFirstSeqNum();

    private:
        std::list<std::shared_ptr<segment>> segments;

        std::mutex segmetsMuxtex;
        Representation *mRep = nullptr;

        int64_t mFirstSeqNum = -1;
        int64_t mLastSeqNum = -1;

        uint64_t mNextStartTime = 0;

        std::vector<std::shared_ptr<segment>> initSegment;

    };
}


#endif //FRAMEWORK_SEGMENTLIST_H
