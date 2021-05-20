//
//  AnalyticsCollectorFactory.h
//  Analytics
//
//  Created by huang_jiafa on 2018/11/07.
//  Copyright (c) 2018 Aliyun. All rights reserved.
//

#ifndef ANALYTICS_COLLECTOR_FACTORY_H
#define ANALYTICS_COLLECTOR_FACTORY_H

#include "AnalyticsQueryListener.h"
#include "IAnalyticsCollector.h"

namespace Cicada {

    class IAnalyticsCollectorFactory {
    public:
        virtual ~IAnalyticsCollectorFactory() {}

        virtual IAnalyticsCollector *createAnalyticsCollector(AnalyticsQueryListener *Listener) = 0;

        virtual void destroyAnalyticsCollector(IAnalyticsCollector *analytics) = 0;
    };

    class AnalyticsCollectorFactory : public IAnalyticsCollectorFactory {
    public:
        static AnalyticsCollectorFactory *Instance(void);

        IAnalyticsCollector *createAnalyticsCollector(AnalyticsQueryListener *Listener) override;

        void destroyAnalyticsCollector(IAnalyticsCollector *analytics) override;

    private:
        AnalyticsCollectorFactory();

        ~AnalyticsCollectorFactory();

    private:
        static AnalyticsCollectorFactory *sInstance;
    };

}// namespace Cicada
#endif // ANALYTICS_COLLECTOR_FACTORY_H

