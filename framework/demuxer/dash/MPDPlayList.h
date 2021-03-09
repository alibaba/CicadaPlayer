//
// Created by yuyuan on 2021/03/08.
//

#ifndef DEMUXER_DASH_MPDPLAYLIST_H
#define DEMUXER_DASH_MPDPLAYLIST_H

#include "ProgramInfo.h"
#include <demuxer/play_list/playList.h>

namespace Cicada {
    namespace Dash {

        enum class ProfileName {
            Unknown,
            Full,
            ISOOnDemand,
            ISOMain,
            ISOLive,
            MPEG2TSMain,
            MPEG2TSSimple,
        };

        class MPDPlayList : public playList {
        public:
            MPDPlayList();

            ~MPDPlayList() override;

            virtual bool isLive() const override;

            virtual bool isLowLatency() const override;

            virtual void InitUtcTime() override;

            virtual int64_t GetUtcTime() const override;

            void setLowLatency(bool b);

            void setProfile(const std::string& urns);

        public:
            bool needsUpdates{false};
            bool lowLatency{false};
            ProgramInfo *programInfo{nullptr};
            ProfileName mProfileName = ProfileName::Unknown;
        };
    }// namespace Dash
}// namespace Cicada


#endif//DEMUXER_DASH_MPDPLAYLIST_H
