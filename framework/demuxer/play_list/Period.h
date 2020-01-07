//
// Created by moqi on 2018/4/25.
//

#ifndef FRAMEWORK_PERIOD_H
#define FRAMEWORK_PERIOD_H

#include <list>
#include "AdaptationSet.h"

using namespace std;

namespace Cicada{
    class playList;

    class AdaptationSet;

    class Period {
    public:
        explicit Period(playList *playlist)
        {
            mPlayList = playlist;
        }

        ~Period();

        void addAdaptationSet(AdaptationSet *adaptSet);

        playList *getPlayList()
        {
            return mPlayList;
        }

        void print();

        std::list<AdaptationSet *> &GetAdaptSets()
        {
            return mAdaptSetList;
        }

    private:
        list<AdaptationSet *> mAdaptSetList;
        int64_t startTime = 0;
        playList *mPlayList = nullptr;
    };
}


#endif //FRAMEWORK_PERIOD_H
