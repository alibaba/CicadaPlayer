//
// Created by yuyuan on 2021/03/09.
//

#ifndef DEMUXER_DASH_SEGMENT_TIMELINE_H
#define DEMUXER_DASH_SEGMENT_TIMELINE_H

#include "InheritablesAttrs.h"
#include <cstddef>
namespace Cicada {
    namespace Dash {
        class SegmentTimeline : public AttrsNode {
            class Element;

        public:
            SegmentTimeline(AttrsNode *parent_);
            virtual ~SegmentTimeline();
            void addElement(uint64_t number, int64_t d, uint64_t r = 0, int64_t t = 0);
            uint64_t getElementNumberByScaledPlaybackTime(int64_t scaled) const;
            bool getScaledPlaybackTimeDurationBySegmentNumber(uint64_t number, int64_t *time, int64_t *duration) const;
            int64_t getScaledPlaybackTimeByElementNumber(uint64_t number) const;
            int64_t getMinAheadScaledTime(uint64_t number) const;
            int64_t getTotalLength() const;
            uint64_t maxElementNumber() const;
            uint64_t minElementNumber() const;
            uint64_t getElementIndexBySequence(uint64_t number) const;
            void pruneByPlaybackTime(int64_t time);
            size_t pruneBySequenceNumber(uint64_t number);
            void updateWith(SegmentTimeline &other);

        private:
            std::list<Element *> elements;
            int64_t totalLength;

            class Element {
            public:
                Element(uint64_t number_, int64_t d_, uint64_t r_, int64_t t_);
                bool contains(int64_t time) const;
                int64_t t;
                int64_t d;
                uint64_t r;
                uint64_t number;
            };
        };
    }// namespace Dash
}// namespace Cicada


#endif//DEMUXER_DASH_SEGMENT_TIMELINE_H
