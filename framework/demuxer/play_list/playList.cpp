//
// Created by moqi on 2018/4/25.
//

#define LOG_TAG "playList"

#include "playList.h"
#include "AdaptationSet.h"
#include "Period.h"
#include "Representation.h"
#include "utils/frame_work_log.h"

namespace Cicada {

    void playList::addPeriod(Period *period)
    {
        mPeriodList.push_back(period);
    }

    void playList::setPlaylistUrl(const std::string &url)
    {
        playlistUrl = url;
    }

    void playList::print()
    {
        AF_LOGD("playlistUrl is %s\n"
                " mDuration is %lld\n,"
                "have %d Periods", playlistUrl.c_str(), mDuration, mPeriodList.size());
    }

    void playList::dump()
    {
        print();
        std::list<Period *> pList;
        std::list<Period *>::iterator i;
        pList = GetPeriods();

        for (i = pList.begin(); i != pList.end(); ++i) {
            Period *p = (*i);
            p->print();
            std::list<AdaptationSet *> aList;
            aList = p->GetAdaptSets();
            std::list<AdaptationSet *>::iterator j;

            for (j = aList.begin(); j != aList.end(); ++j) {
                AdaptationSet *a = (*j);
                a->print();
                std::list<Representation *> rList;
                rList = a->getRepresentations();
                std::list<Representation *>::iterator k;

                for (k = rList.begin(); k != rList.end(); k++) {
                    Representation *r = (*k);
                    r->print();
                }
            }
        }
    }

    bool playList::isLive() const
    {
        return false;
    }

    bool playList::isLowLatency() const
    {
        return false;
    }

    Dash::DashUrl playList::getUrlSegment() const
    {
        Dash::DashUrl ret;

        if (!baseUrls.empty()) ret = Dash::DashUrl(baseUrls.front());

        if (!ret.hasScheme() && !playlistUrl.empty()) {
            ret.prepend(Dash::DashUrl(playlistUrl));
        }

        return ret;
    }

    void playList::addBaseUrl (const std::string &url)
    {
        baseUrls.push_back(url);
    }

    const std::string &playList::getPlaylistUrl()
    {
        return playlistUrl;
    }

    playList::~playList()
    {
        std::list<Period *>::iterator i;
        AF_LOGD("mPeriodList size is %d", mPeriodList.size());

        while (!mPeriodList.empty()) {
            i = mPeriodList.begin();
            Period *p = (*i);
            delete p;
            mPeriodList.erase(i);
        }
    }

    void playList::updateWith(playList *updatedPlaylist)
    {
        if (updatedPlaylist == nullptr) {
            return;
        }
        availabilityEndTime = updatedPlaylist->availabilityEndTime;
        baseUrls = updatedPlaylist->baseUrls;

        auto itP1 = mPeriodList.begin();
        auto itP2 = updatedPlaylist->mPeriodList.begin();
        for (; itP1 != mPeriodList.end() && itP2 != updatedPlaylist->mPeriodList.end();) {
            (*itP1)->updateWith(*itP2);
            itP1++;
            itP2++;
        }
    }
}