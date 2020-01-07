//
//  AnalyticsQueryListener.h
//  Analytics
//
//  Created by huang_jiafa on 2018/11/07.
//  Copyright (c) 2018 Aliyun. All rights reserved.
//

#ifndef ANALYTICS_QUERY_LISTENER_H
#define ANALYTICS_QUERY_LISTENER_H

#include <string>
#include "native_cicada_player_def.h"

namespace Cicada {
    class AnalyticsQueryListener {
    public:
        virtual ~AnalyticsQueryListener() = default;

        virtual int64_t OnAnalyticsGetCurrentPosition() = 0;

        virtual int64_t OnAnalyticsGetBufferedPosition() = 0;
        
        virtual int64_t OnAnalyticsGetDuration() = 0;

        virtual std::string OnAnalyticsGetPropertyString(PropertyKey key) = 0;

        //virtual int64_t OnAnalyticsGetPropertyLong(int key, int64_t defaultValue) = 0;
    };

}// namespace Cicada

#endif // AnalyticsQueryListener_H

