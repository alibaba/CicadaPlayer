//
// Created by moqi on 2018/4/25.
//

#include "Period.h"
#include "AdaptationSet.h"
#include "utils/frame_work_log.h"

#define LOG_TAG "Period"

namespace Cicada {

    Period::Period(playList *playlist) : Dash::SegmentInformation(playlist)
    {
        mPlayList = playlist;
    }

    void Period::addAdaptationSet(AdaptationSet *adaptSet)
    {
        mAdaptSetList.push_back(adaptSet);
    }

    void Period::print()
    {
        AF_LOGD("startTime is %lld\n"
                "have %d AdaptSets\n",
                startTime, mAdaptSetList.size());
    }

    Period::~Period()
    {
        for (auto &it : mAdaptSetList) {
            delete it;
        }

        mAdaptSetList.clear();
    }
}
