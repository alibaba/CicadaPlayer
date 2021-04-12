//
// Created by yuyuan on 2021/04/12.
//

#ifndef DEMUXER_DASH_UTCTIMING_H
#define DEMUXER_DASH_UTCTIMING_H
#include <string>

namespace Cicada {
    namespace Dash {

        enum UTCTimingType {
            UTCTimingUnknown,

            //urn:mpeg:dash:utc:ntp:2014
            UTCTimingNtp,

            //urn:mpeg:dash:utc:sntp:2014
            UTCTimingSntp,

            //urn:mpeg:dash:utc:http-head:2014
            UTCTimingHttpHead,

            //urn:mpeg:dash:utc:http-xsdate:2014
            UTCTimingXsdate,

            //urn:mpeg:dash:utc:http-iso:2014
            UTCTimingHttpIso,

            //urn:mpeg:dash:utc:http-ntp:2014
            UTCTimingHttpNtp,

            //urn:mpeg:dash:utc:direct:2014
            UTCTimingDirect,
        };

        class UTCTiming {
        public:
            UTCTiming(const std::string &schemeId, const std::string value);
            ~UTCTiming();
            void SetSchemeId(const std::string &schemeId);

        public:
            UTCTimingType mUtcType = UTCTimingUnknown;
            std::string mValue;
        };
    }// namespace Dash
}// namespace Cicada


#endif//DEMUXER_DASH_UTCTIMING_H
