//
// Created by moqi on 2018/4/25.
//

#include "Period.h"

#define LOG_TAG "Period"

#include <utils/frame_work_log.h>

namespace Cicada {
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
