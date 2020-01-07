//
//  AnalyticsType.h
//  Analytics
//
//  Created by huang_jiafa on 2018/11/07.
//  Copyright (c) 2018 Aliyun. All rights reserved.
//

#ifndef ANALYTICS_TYPE_H
#define ANALYTICS_TYPE_H

#include "native_cicada_player_def.h"

namespace Cicada {

    class QualityInfo {
    public:
        bool isAuto;
        int64_t beforeBitrate;
        int64_t afterBitrate;

        int64_t beforeDefinition;
        int64_t afterDefinition;
    };

    class AudioInfo {
    public:

    };

    class SubtitleInfo {
    public:

    };
}// namespace Cicada
#endif // ANALYTICS_TYPE_H
