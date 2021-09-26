//
// Created by moqi on 2018/11/30.
//
#include <utils/CicadaJSON.h>
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
        bClearShowWhenStop = false;
        bEnableTunnelRender = false;
        pixelBufferOutputFormat = 0;
        liveStartIndex = -3;
        mDisableAudio = false;
        mDisableVideo = false;
        mPositionTimerIntervalMs = 500;
        mMaxBackwardBufferDuration = 0;
        preferAudio = false;
    }

    std::string MediaPlayerConfig::toString() const
    {
        CicadaJSONItem item{};
        item.addValue("referer", referer);
        item.addValue("httpProxy", httpProxy);
        item.addValue("userAgent", userAgent);
        item.addValue("networkTimeout", networkTimeout);
        item.addValue("networkRetryCount", networkRetryCount);
        item.addValue("maxDelayTime", maxDelayTime);
        item.addValue("maxBufferDuration", maxBufferDuration);
        item.addValue("highBufferDuration", highBufferDuration);
        item.addValue("startBufferDuration", startBufferDuration);
        item.addValue("bClearShowWhenStop", bClearShowWhenStop);
        item.addValue("bEnableTunnelRender", bEnableTunnelRender);
        item.addValue("mDisableAudio", mDisableAudio);
        item.addValue("mDisableVideo", mDisableVideo);
        item.addValue("mPositionTimerIntervalMs", mPositionTimerIntervalMs);
        item.addValue("mMaxBackwardBufferDuration", (double) mMaxBackwardBufferDuration);
        item.addValue("preferAudio", preferAudio);
        return item.printJSON();
    }
}
