//
// Created by yuyuan on 2021/03/09.
//

#ifndef DEMUXER_DASH_SEGMENT_BASE_H
#define DEMUXER_DASH_SEGMENT_BASE_H

#include "DashSegment.h"
#include "ISegmentBase.h"
#include "InheritablesAttrs.h"

namespace Cicada {
    namespace Dash {

        class SegmentBase : public DashSegment, public ISegmentBase {
        public:
            SegmentBase(SegmentInformation *parent = nullptr);
            virtual ~SegmentBase();

            virtual int64_t getMinAheadTime(uint64_t curnum) const override;
            virtual DashSegment *getMediaSegment(uint64_t number) const override;
            virtual DashSegment *getNextMediaSegment(uint64_t i_pos, uint64_t *pi_newpos, bool *pb_gap) const override;
            virtual uint64_t getStartSegmentNumber() const override;

            virtual bool getSegmentNumberByTime(int64_t time, uint64_t *ret) const override;
            virtual bool getPlaybackTimeDurationBySegmentNumber(uint64_t number, int64_t *time, int64_t *duration) const override;
        };


    }// namespace Dash
}// namespace Cicada


#endif//DEMUXER_DASH_SEGMENT_BASE_H
