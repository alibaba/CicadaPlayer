//
// Created by yuyuan on 2021/03/15.
//

#ifndef DEMUXER_DASH_ISEGMENT_BASE_H
#define DEMUXER_DASH_ISEGMENT_BASE_H

#include "InheritablesAttrs.h"
#include <vector>

namespace Cicada {
    namespace Dash {
        class DashSegment;
        class SegmentInformation;

        class ISegmentBase : public AttrsNode {
        public:
            ISegmentBase(SegmentInformation *parent, Type type);
            ~ISegmentBase();

            virtual int64_t getMinAheadTime(uint64_t curnum) const = 0;
            virtual DashSegment *getMediaSegment(uint64_t pos) const = 0;
            virtual DashSegment *getInitSegment() const;
            virtual DashSegment *getIndexSegment() const;
            virtual void setInitSegment(DashSegment *segment);
            virtual void setIndexSegment(DashSegment *segment);
            virtual DashSegment *getNextMediaSegment(uint64_t i_pos, uint64_t *pi_newpos, bool *pb_gap) const = 0;
            virtual uint64_t getStartSegmentNumber() const = 0;
            virtual void updateWith(ISegmentBase *updated, bool restamp = false);

            virtual bool getSegmentNumberByTime(int64_t time, uint64_t *ret) const = 0;
            virtual bool getPlaybackTimeDurationBySegmentNumber(uint64_t number, int64_t *time, int64_t *duration) const = 0;

            static DashSegment *findSegmentByScaledTime(const std::vector<DashSegment *> &segments, int64_t time);
            static uint64_t findSegmentNumberByScaledTime(const std::vector<DashSegment *> &segments, int64_t time);

        protected:
            SegmentInformation *mParent = nullptr;
            DashSegment *mInitSegment = nullptr;
            DashSegment *mIndexSegment = nullptr;
        };


    }// namespace Dash
}// namespace Cicada


#endif//DEMUXER_DASH_ISEGMENT_BASE_H
