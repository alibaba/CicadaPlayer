//
// Created by yuyuan on 2021/04/12.
//

#include "UTCTiming.h"

using namespace Cicada::Dash;

UTCTiming::UTCTiming(const std::string &schemeId, const std::string value) : mValue(value)
{
    SetSchemeId(schemeId);
}

UTCTiming::~UTCTiming()
{}

void UTCTiming::SetSchemeId(const std::string &schemeId)
{
    if (schemeId == "urn:mpeg:dash:utc:ntp:2014") {
        mUtcType = UTCTimingNtp;
    } else if (schemeId == "urn:mpeg:dash:utc:sntp:2014") {
        mUtcType = UTCTimingSntp;
    } else if (schemeId == "urn:mpeg:dash:utc:http-head:2014") {
        mUtcType = UTCTimingHttpHead;
    } else if (schemeId == "urn:mpeg:dash:utc:http-xsdate:2014") {
        mUtcType = UTCTimingXsdate;
    } else if (schemeId == "urn:mpeg:dash:utc:http-iso:2014") {
        mUtcType = UTCTimingHttpIso;
    } else if (schemeId == "urn:mpeg:dash:utc:http-ntp:2014") {
        mUtcType = UTCTimingHttpNtp;
    } else if (schemeId == "urn:mpeg:dash:utc:direct:2014") {
        mUtcType = UTCTimingDirect;
    }
}