//
//  analytics_utils.h
//  Analytics
//
//  Created by huang_jiafa on 2018/11/07.
//  Copyright (c) 2018 Aliyun. All rights reserved.
//

#ifndef ANALYTICS_UTILS_H
#define ANALYTICS_UTILS_H

#include <string>
#include "AnalyticsType.h"

namespace Cicada {

    class AnalyticsUtils {
    public:
        AnalyticsUtils();

        ~AnalyticsUtils();

        static int64_t getTimeMS();

        static std::string getURLEncodeString(const std::string &s);
    };

}// namespace Cicada

#endif // ANALYTICS_UTILS_H

