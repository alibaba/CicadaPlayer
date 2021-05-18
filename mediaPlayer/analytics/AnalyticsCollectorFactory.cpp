//
//  AnalyticsCollectorFactory.cpp
//  Analytics
//
//  Created by huang_jiafa on 2018/11/07.
//  Copyright (c) 2018 Aliyun. All rights reserved.
//

#include "AnalyticsCollectorFactory.h"

#include "AnalyticsCollectorImpl.h"
#include <mutex>

namespace Cicada {
    AnalyticsCollectorFactory AnalyticsCollectorFactory::sInstance{};
    AnalyticsCollectorFactory::AnalyticsCollectorFactory() = default;

    AnalyticsCollectorFactory::~AnalyticsCollectorFactory() = default;

    AnalyticsCollectorFactory *AnalyticsCollectorFactory::Instance()
    {
        return &sInstance;
    }

    IAnalyticsCollector *AnalyticsCollectorFactory::createAnalyticsCollector(AnalyticsQueryListener *Listener)
    {
        return new AnalyticsCollectorImpl(Listener);
    }

    void AnalyticsCollectorFactory::destroyAnalyticsCollector(IAnalyticsCollector *analytics)
    {
        delete analytics;
    }
}// namespace Cicada
