//
// Created by yuyuan on 2021/03/09.
//

#ifndef DEMUXER_DASH_SEGMENT_LIST_H
#define DEMUXER_DASH_SEGMENT_LIST_H

#include "ISegmentBase.h"

namespace Cicada {
    namespace Dash {
        class SegmentList : public ISegmentBase {
        public:
            SegmentList(SegmentInformation *parent = nullptr);
            virtual ~SegmentList();

            const std::vector<DashSegment *> &getSegments() const;
            void addSegment(DashSegment *seg);
            virtual void updateWith(ISegmentBase *updated, bool restamp = false) override;
            void pruneBySegmentNumber(uint64_t tobelownum);
            void pruneByPlaybackTime(int64_t time);
            int64_t getTotalLength() const;

            virtual int64_t getMinAheadTime(uint64_t curnum) const override;
            virtual DashSegment *getMediaSegment(uint64_t pos) const override;
            virtual DashSegment *getNextMediaSegment(uint64_t i_pos, uint64_t *pi_newpos, bool *pb_gap) const override;
            virtual uint64_t getStartSegmentNumber() const override;
            virtual bool getSegmentNumberByTime(int64_t time, uint64_t *ret) const override;
            virtual bool getPlaybackTimeDurationBySegmentNumber(uint64_t number, int64_t *time, int64_t *duration) const override;

        private:
            std::vector<DashSegment *> segments;
            int64_t totalLength;
        };
    }// namespace Dash
}// namespace Cicada


#endif//DEMUXER_DASH_SEGMENT_LIST_H
