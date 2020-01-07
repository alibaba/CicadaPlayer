//
//  IAnalyticsRender.h
//  Analytics
//
//  Created by huang_jiafa on 2018/11/07.
//  Copyright (c) 2018 Aliyun. All rights reserved.
//

#ifndef IANALYTICS_RENDER_H
#define IANALYTICS_RENDER_H

namespace Cicada {
    class IAnalyticRender {
    public:
        virtual ~IAnalyticRender() = default;

        virtual void ReportFirstRender() = 0;

        virtual void ReportFirstVideoRender() = 0;

        virtual void ReportFirstAudioRender() = 0;

        virtual void ReportVideoSizeChanged(int width, int height) = 0;

        virtual void ReportSnapshot() = 0;
    };

}// namespace Cicada
#endif // IANALYTICS_RENDER_H

