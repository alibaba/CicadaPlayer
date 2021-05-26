//
// Created by moqi on 2018/4/25.
//

#ifndef FRAMEWORK_PLAYLIST_H
#define FRAMEWORK_PLAYLIST_H

#include "demuxer/dash/DashUrl.h"
#include "demuxer/dash/IDashUrl.h"
#include <list>
#include <utils/UTCTimer.h>

//using namespace std;
namespace Cicada{
    class Period;
    class playList : public Dash::IDashUrl {
    public:
        playList()
        {

        }

        virtual ~playList();

        void addPeriod(Period *period);

        void setPlaylistUrl(const std::string &);

        const std::string &getPlaylistUrl();

        void print();

        std::list<Period *> &GetPeriods()
        {
            return mPeriodList;
        };

        void setDuration(int64_t duration)
        {
            mDuration = duration;
        }

        int64_t getDuration() const
        {
            return mDuration;
        }

        void dump();

        virtual bool isLive() const;

        virtual bool isLowLatency() const;

        virtual Dash::DashUrl getUrlSegment() const override;

        void addBaseUrl (const std::string &url);

        void updateWith(playList *updatedPlaylist);

    public:
        int64_t minUpdatePeriod{0};
        int64_t maxSegmentDuration{0};
        std::string type;
        int64_t minBufferTime{0};
        int64_t maxBufferTime{0};
        int64_t availabilityStartTime{0};
        int64_t availabilityEndTime{0};
        int64_t timeShiftBufferDepth{0};
        int64_t suggestedPresentationDelay{0};

    protected:
        UTCTimer *mUtcTimer = nullptr;

    private:
        std::list<Period *> mPeriodList{};
        int64_t mDuration = 0;
        std::string playlistUrl = "";
        std::vector<std::string> baseUrls;

    };
}


#endif //FRAMEWORK_PLAYLIST_H
