//
// Created by moqi on 2018/4/25.
//

#define LOG_TAG "Period"
#include "Period.h"
#include "AdaptationSet.h"
#include "utils/frame_work_log.h"

using namespace Cicada;

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

int64_t Period::getPeriodStart() const
{
    return startTime;
}

int64_t Period::getPeriodDuration() const
{
    return duration;
}
