//
// Created by yuyuan on 2021/03/09.
//

#ifndef DEMUXER_DASH_SEGMENT_INFORMATION_H
#define DEMUXER_DASH_SEGMENT_INFORMATION_H

#include "IDashUrl.h"
#include "InheritablesAttrs.h"
#include "demuxer/play_list/playList.h"
#include <memory>
#include <vector>
namespace Cicada {

    class playList;

    namespace Dash {
        class DashSegment;
        class ISegmentBase;

        class SegmentInformation : public IDashUrl, public AttrsNode {
        public:
            SegmentInformation(SegmentInformation *parent = 0);
            SegmentInformation(playList *parent_);

            virtual ~SegmentInformation();

            void setID(const std::string &id);
            std::string getID() const;

            virtual int64_t getPeriodStart() const;
            virtual int64_t getPeriodDuration() const;

            class SplitPoint {
            public:
                size_t offset;
                int64_t time;
                int64_t duration;
            };
            void SplitUsingIndex(std::vector<SplitPoint> &splitlist);

            virtual DashSegment *getInitSegment() const;
            virtual DashSegment *getIndexSegment() const;
            virtual DashSegment *getMediaSegment(uint64_t pos = 0) const;
            virtual DashSegment *getNextMediaSegment(uint64_t i_pos, uint64_t *pi_newpos, bool *pb_gap) const;

            virtual void updateWith(SegmentInformation *updated);
            virtual void pruneBySegmentNumber(uint64_t num);
            virtual void pruneByPlaybackTime(int64_t time);

            virtual DashUrl getUrlSegment() const override;
            std::unique_ptr<DashUrl> baseUrl = nullptr;

            ISegmentBase *getProfile() const;
            void updateSegmentList(SegmentList *list, bool restamp = false);
            void setSegmentTemplate(SegmentTemplate *templ);
            const ISegmentBase *inheritSegmentProfile() const;
            int64_t getMediaSegmentStartTime(uint64_t i_pos) const;

            virtual playList *getPlayList() const;

        protected:
            std::vector<SegmentInformation *> childs;
            SegmentInformation *getChildByID(const std::string &id);
            SegmentInformation *mParent;
            std::string mId;
        };

    }// namespace Dash
}// namespace Cicada


#endif//DEMUXER_DASH_SEGMENT_INFORMATION_H
