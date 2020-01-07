//
// Created by moqi on 2018/4/25.
//
#define LOG_TAG "AdaptationSet"

#include <utils/frame_work_log.h>
#include "AdaptationSet.h"

namespace Cicada {

    void AdaptationSet::addRepresentation(Representation *represent)
    {
        mRepresentList.push_back(represent);
    }

    void AdaptationSet::print()
    {
        AF_LOGD("%s have %d Represens\n", mDescription.c_str(), mRepresentList.size());
    }

    AdaptationSet::~AdaptationSet()
    {
        for (auto it = mRepresentList.begin(); it != mRepresentList.end(); ++it) {
            delete *it;
        }

        mRepresentList.clear();
    }
}