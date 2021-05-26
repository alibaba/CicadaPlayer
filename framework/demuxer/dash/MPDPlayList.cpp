//
// Created by yuyuan on 2021/03/08.
//

#include "MPDPlayList.h"
#include "UTCTiming.h"
#include "data_source/dataSourcePrototype.h"
#include "utils/UTCTimer.h"
#include "utils/frame_work_log.h"
#include "utils/timer.h"

using namespace Cicada::Dash;

MPDPlayList::MPDPlayList()
{}

MPDPlayList::~MPDPlayList()
{
    delete mUtcTiming;
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
    std::string utcTime;
    if (mUtcTiming) {
        if (mUtcTiming->mUtcType == UTCTimingDirect) {
            utcTime = mUtcTiming->mValue;
        } else if (mUtcTiming->mUtcType == UTCTimingNtp) {
            NTPClient ntpClient(mUtcTiming->mValue);
            ntpClient.getTimeSync(5000);
            utcTime = (std::string) ntpClient;
        } else if (mUtcTiming->mUtcType == UTCTimingXsdate || mUtcTiming->mUtcType == UTCTimingHttpIso) {
            std::string url = mUtcTiming->mValue;
            IDataSource *dataSource = dataSourcePrototype::create(url, nullptr);
            dataSource->Open(0);

            int size = dataSource->Seek(0, SEEK_SIZE);
            auto *buffer = static_cast<char *>(malloc(size));
            int len = 0;
            while (len < size) {
                int ret = dataSource->Read(buffer + len, size - len);
                if (ret > 0) {
                    len += ret;
                } else {
                    break;
                }
            }
            if (len > 0) {
                utcTime.assign(buffer, len);
            }
            free(buffer);
            delete dataSource;
        }
    }
    if (utcTime.empty()) {
        AF_LOGI("[dash] get utc time in mpd failed, use default ntp server");
        NTPClient ntpClient;
        ntpClient.getTimeSync(5000);
        utcTime = (std::string) ntpClient;
    }
    if (utcTime.empty()) {
        af_init_utc_time_ms(af_gettime_ms());
        AF_LOGW("[dash] get utc time failed, use local time");
    } else {
        af_init_utc_time(utcTime.c_str());
    }
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