//
// Created by moqi on 2018/4/25.
//

#ifndef FRAMEWORK_PERIOD_H
#define FRAMEWORK_PERIOD_H

#include <list>
#include "demuxer/dash/SegmentInformation.h"

using namespace std;

namespace Cicada{

    class playList;
    class AdaptationSet;

    class Period : public Dash::SegmentInformation {
    public:
        explicit Period(playList *playlist);
        ~Period();

        void addAdaptationSet(AdaptationSet *adaptSet);

        playList *getPlayList() const override
        {
            return mPlayList;
        }

        void print();

        std::list<AdaptationSet *> &GetAdaptSets()
        {
            return mAdaptSetList;
        }

    public:
        int64_t startTime{0};
        int64_t duration{0};

    private:
        list<AdaptationSet *> mAdaptSetList;
        
        playList *mPlayList = nullptr;

        

    };
}


#endif //FRAMEWORK_PERIOD_H
