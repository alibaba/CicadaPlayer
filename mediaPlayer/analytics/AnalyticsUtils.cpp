//
//  analytics_utils.cpp
//  Analytics
//
//  Created by huang_jiafa on 2018/11/07.
//  Copyright (c) 2018 Aliyun. All rights reserved.
//

#include "AnalyticsUtils.h"
#include <sstream>
#include "utils/timer.h"

namespace Cicada {
    AnalyticsUtils::AnalyticsUtils() {

    }

    AnalyticsUtils::~AnalyticsUtils() {

    }

    int64_t AnalyticsUtils::getTimeMS() {
        return af_gettime() / 1000;
    }

    std::string AnalyticsUtils::getURLEncodeString(const std::string &original) {
        static const char dictionary[] = "0123456789abcdef";
        std::stringstream stream;
        for (long i = 0; i < original.length(); ++i) {
            const char &c = original[i];
            if ((48 <= c && c <= 57) //0-9
                || (65 <= c && c <= 90) //abc...xyz
                || (97 <= c && c <= 122) //ABC...XYZ
                || (c == '-' || c == '_' || c == '.' || c == '~')) {
                stream << c;
            } else {
                stream << '%';
                stream << dictionary[(c & 0xF0) >> 4];
                stream << dictionary[(c & 0x0F)];
            }
        }
        return stream.str();
    }
}// namespace Cicada
