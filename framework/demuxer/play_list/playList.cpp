//
// Created by moqi on 2018/4/25.
//

#define LOG_TAG "playList"

#include <utils/frame_work_log.h>
#include "playList.h"


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
}