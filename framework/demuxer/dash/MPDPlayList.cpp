//
// Created by yuyuan on 2021/03/08.
//

#include "MPDPlayList.h"
#include "utils/UTCTimer.h"

using namespace Cicada::Dash;

MPDPlayList::MPDPlayList()
{}

MPDPlayList::~MPDPlayList()
{
    delete mUtcTimer;
}

bool MPDPlayList::isLive() const
{
    if (type.empty()) {
        return mProfileName == ProfileName::ISOLive;
    } else {
        return (type != "static");
    }
}

bool MPDPlayList::isLowLatency() const
{
    return lowLatency;
}

void MPDPlayList::setLowLatency(bool b)
{
    lowLatency = b;
}

void MPDPlayList::InitUtcTime()
{
    // TODO: use ntp server address in mpd instead of default
    NTPClient ntpClient;
    ntpClient.getTimeSync();
    std::string utcTime = (std::string)ntpClient;
    mUtcTimer = new UTCTimer(utcTime);
    mUtcTimer->start();
}

int64_t MPDPlayList::GetUtcTime() const
{
    int64_t utcTime = 0;
    if (mUtcTimer) {
        utcTime = mUtcTimer->get();
        std::string t = (std::string)(*mUtcTimer);
    }
    return utcTime;
}

void MPDPlayList::setProfile(const std::string &urns)
{
    const std::vector<std::pair<ProfileName, std::string>> urnmap = {
            {ProfileName::Full, "urn:mpeg:dash:profile:full:2011"},
            {ProfileName::ISOOnDemand, "urn:mpeg:dash:profile:isoff-on-demand:2011"},
            {ProfileName::ISOOnDemand, "urn:mpeg:mpegB:profile:dash:isoff-basic-on-demand:cm"},
            {ProfileName::ISOOnDemand, "urn:mpeg:dash:profile:isoff-ondemand:2011"},
            {ProfileName::ISOMain, "urn:mpeg:dash:profile:isoff-main:2011"},
            {ProfileName::ISOLive, "urn:mpeg:dash:profile:isoff-live:2011"},
            {ProfileName::MPEG2TSMain, "urn:mpeg:dash:profile:mp2t-main:2011"},
            {ProfileName::MPEG2TSSimple, "urn:mpeg:dash:profile:mp2t-simple:2011"},
    };
    if (urns.size() > 0) {
        size_t pos;
        size_t nextpos = std::string::npos;
        do {
            pos = nextpos + 1;
            nextpos = urns.find_first_of(",", pos);
            std::string urn = urns.substr(pos, nextpos - pos);
            for (auto &itUrn : urnmap) {
                if (itUrn.second == urn) {
                    mProfileName = itUrn.first;
                    break;
                }
            }
        } while (nextpos != std::string::npos && mProfileName == ProfileName::Unknown);
    }
}