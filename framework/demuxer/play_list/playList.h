//
// Created by moqi on 2018/4/25.
//

#ifndef FRAMEWORK_PLAYLIST_H
#define FRAMEWORK_PLAYLIST_H

#include <list>
#include "Period.h"

//using namespace std;
namespace Cicada{
    class Period;

    class playList {
    public:
        playList()
        {

        }

        ~playList();

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

        int64_t getDuration()
        {
            return mDuration;
        }

        void dump();

    private:
        std::list<Period *> mPeriodList{};
        int64_t mDuration = 0;
        std::string playlistUrl = "";


    };
}


#endif //FRAMEWORK_PLAYLIST_H
