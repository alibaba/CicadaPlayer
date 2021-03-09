//
// Created by moqi on 2018/4/25.
//
#define LOG_TAG "AdaptationSet"

#include "AdaptationSet.h"
#include "Period.h"
#include "Representation.h"
#include "utils/frame_work_log.h"

namespace Cicada {

    AdaptationSet::AdaptationSet(Period *period) : Dash::SegmentInformation(period)
    {
        mPeriod = period;
    }

    void AdaptationSet::addRepresentation(Representation *represent)
    {
        mRepresentList.push_back(represent);
    }

    void AdaptationSet::print()
    {
        AF_LOGD("%s have %d Represens\n", mDescription.c_str(), mRepresentList.size());
    }

    const std::string &AdaptationSet::getMimeType() const
    {
        return mimeType;
    }

    AdaptationSet::~AdaptationSet()
    {
        for (auto it = mRepresentList.begin(); it != mRepresentList.end(); ++it) {
            delete *it;
        }

        mRepresentList.clear();
    }
}