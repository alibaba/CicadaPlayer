//
//  I_analytics_collector.h
//  Analytics
//
//  Created by huang_jiafa on 2018/11/07.
//  Copyright (c) 2018 Aliyun. All rights reserved.
//

#ifndef IANALYTICS_COLLECTOR_H
#define IANALYTICS_COLLECTOR_H

#include "IAnalyticsPlayer.h"
#include "IAnalyticsRender.h"
#include "IAnalyticsSource.h"
#include "AnalyticsCollectorListener.h"
#include "AnalyticsQueryListener.h"

namespace Cicada {
    class ICollectorData {
    public:
        // decale as pure function to avoid create objects
        // getAnalyticsData will return the object of derived class
        virtual ~ICollectorData() = 0;
    };

    class IAnalyticsCollector : public IAnalyticPlayer, public IAnalyticSource,
                                       public IAnalyticRender {
    public:
        IAnalyticsCollector(AnalyticsQueryListener *queryListener)
        {
            mQueryListener = queryListener;
        };

        virtual ~IAnalyticsCollector() = default;

        virtual AnalyticsQueryListener *GetQueryListener() { return mQueryListener; };

        virtual void AddListener(AnalyticsCollectorListener *listener) {};

        virtual void RemoveListener(AnalyticsCollectorListener *listener) {};

        virtual const ICollectorData *getAnalyticsData() = 0;



    protected:
        AnalyticsQueryListener *mQueryListener = nullptr;
    };

}// namespace Cicada

#endif // IANALYTICS_COLLECTOR_H

