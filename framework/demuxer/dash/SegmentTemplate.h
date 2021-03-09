//
// Created by yuyuan on 2021/03/09.
//

#ifndef DEMUXER_DASH_SEGMENT_TEMPLATE_H
#define DEMUXER_DASH_SEGMENT_TEMPLATE_H

#include "DashSegment.h"
#include "ISegmentBase.h"

namespace Cicada {
    namespace Dash {
        class IDashUrl;

        class SegmentTemplateSegment : public DashSegment {
        public:
            SegmentTemplateSegment(IDashUrl *parent = nullptr);
            virtual ~SegmentTemplateSegment();
            void setSourceUrl(const std::string &url) override;
            void setParentTemplate(SegmentTemplate *templ_);

        protected:
            const SegmentTemplate *templ;
        };

        class SegmentTemplate : public ISegmentBase {
        public:
            SegmentTemplate(SegmentTemplateSegment *seg, SegmentInformation *parent = nullptr);
            virtual ~SegmentTemplate();
            void setSourceUrl(const std::string &url);
            uint64_t getLiveTemplateNumber(int64_t playbacktime, bool abs = true) const;
            void pruneByPlaybackTime(int64_t time);
            size_t pruneBySequenceNumber(uint64_t number);

            virtual int64_t getMinAheadTime(uint64_t curnum) const override;
            virtual DashSegment *getMediaSegment(uint64_t number) const override;
            virtual DashSegment *getNextMediaSegment(uint64_t i_pos, uint64_t *pi_newpos, bool *pb_gap) const override;
            virtual DashSegment *getInitSegment() const override;
            virtual uint64_t getStartSegmentNumber() const override;

            virtual bool getSegmentNumberByTime(int64_t time, uint64_t *ret) const override;
            virtual bool getPlaybackTimeDurationBySegmentNumber(uint64_t number, int64_t *time, int64_t *duration) const override;

        protected:
            SegmentInformation *parentSegmentInformation;
            SegmentTemplateSegment *virtualsegment;
        };

        class SegmentTemplateInit : public DashSegment {
        public:
            SegmentTemplateInit(SegmentTemplate *templ_, IDashUrl *parent = nullptr);
            virtual ~SegmentTemplateInit();
            virtual void setSourceUrl(const std::string &url) override;

        protected:
            const SegmentTemplate *templ;
        };
    }// namespace Dash
}// namespace Cicada


#endif//DEMUXER_DASH_SEGMENT_TEMPLATE_H
