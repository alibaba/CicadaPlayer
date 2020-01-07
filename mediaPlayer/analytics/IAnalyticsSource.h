//
//  IAnalyticsSource.h
//  Analytics
//
//  Created by huang_jiafa on 2018/11/07.
//  Copyright (c) 2018 Aliyun. All rights reserved.
//

#ifndef IANALYTICS_SOURCE_H
#define IANALYTICS_SOURCE_H

#include <string>
#include "AnalyticsType.h"

namespace Cicada {
    class IAnalyticSource {
    public:
        virtual ~IAnalyticSource() = default;

        // source
        virtual void ReportPrepare() = 0;

        virtual void ReportPrepared() = 0;

        virtual void ReportCompletion() = 0;

        // status
        virtual void ReportLoadingStart() = 0;

        virtual void ReportLoadingEnd() = 0;

        virtual void ReportLoopingStart() = 0;
        
        // Track & selection
        virtual void ReportThumbnailURL(const std::string &URL) = 0;

        virtual void ReportThumbnailReady(bool success) = 0;

        virtual void ReportCurrentBitrate(int bitrate) = 0;

        // other

        virtual void ReportBlackInfo() = 0;

        virtual void ReportSwitchToSoftDecode() = 0;
    };

}// namespace Cicada
#endif // IANALYTICS_SOURCE_H

