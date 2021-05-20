//
//  AnalyticsCollectorFactory.cpp
//  Analytics
//
//  Created by huang_jiafa on 2018/11/07.
//  Copyright (c) 2018 Aliyun. All rights reserved.
//

#include "AnalyticsCollectorFactory.h"

#include <mutex>
#include "AnalyticsCollectorImpl.h"

namespace Cicada {
    AnalyticsCollectorFactory *AnalyticsCollectorFactory::sInstance = NULL;

    AnalyticsCollectorFactory::AnalyticsCollectorFactory()
    {

    }

    AnalyticsCollectorFactory::~AnalyticsCollectorFactory()
    {

    }

    AnalyticsCollectorFactory *AnalyticsCollectorFactory::Instance(void)
    {
        static std::once_flag oc;
        std::call_once(oc, [&] {
            sInstance = new AnalyticsCollectorFactory();
        });

        return sInstance;
    }

    IAnalyticsCollector *
    AnalyticsCollectorFactory::createAnalyticsCollector(AnalyticsQueryListener *Listener)
    {
        return new AnalyticsCollectorImpl(Listener);
    }

    void
    AnalyticsCollectorFactory::destroyAnalyticsCollector(IAnalyticsCollector *analytics)
    {
        delete analytics;
    }
}// namespace Cicada


