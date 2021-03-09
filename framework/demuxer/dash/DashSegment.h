//
// Created by yuyuan on 2021/03/15.
//

#ifndef DEMUXER_DASH_DASH_SEGMENT_H
#define DEMUXER_DASH_DASH_SEGMENT_H

#include "DashUrl.h"
#include "IDashUrl.h"
#include <vector>

namespace Cicada {
    namespace Dash {
        class DashSegment : public IDashUrl {
        public:
            DashSegment(IDashUrl *parent);
            DashSegment(IDashUrl *parent, int64_t start, int64_t end);
            ~DashSegment();

            void setByteRange(int64_t start, int64_t end);
            virtual void setSourceUrl(const std::string &url);
            DashUrl getUrlSegment() const override;
            size_t getOffset() const;
            bool contains(size_t byte) const;
            void addSubSegment(DashSegment *subsegment);
            void setSequenceNumber(uint64_t seq);
            uint64_t getSequenceNumber() const;
            int compare(DashSegment *other) const;

        public:
            int64_t startTime = 0;
            int64_t duration = 0;
            bool discontinuity = false;
            uint64_t sequence = 0;
            int64_t startByte{INT64_MIN};
            int64_t endByte{INT64_MIN};
            std::vector<DashSegment *> subsegments;
            bool isInitSegment = false;

        protected:
            DashUrl sourceUrl;
            bool templated = false;
        };
    }// namespace Dash
}// namespace Cicada


#endif//DEMUXER_DASH_DASH_SEGMENT_H
