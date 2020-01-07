//
// Created by moqi on 2018/11/30.
//
#include "MediaPlayerConfig.h"

namespace Cicada {
    MediaPlayerConfig::MediaPlayerConfig()
    {
        referer = "";
        httpProxy = "";
        userAgent = "";
        networkTimeout = 15000;
        networkRetryCount = 2;
        maxDelayTime = 5000;
        maxBufferDuration = 50000;
        highBufferDuration = 3000;
        startBufferDuration = 500;
        maxProbeSize = -1;
        bClearShowWhenStop = false;
        bEnableTunnelRender = false;
    }
}
