//
// Created by moqi on 2018/9/21.
//

#include "player_types.h"

#if __APPLE__
    #include <TargetConditionals.h>
#endif


namespace Cicada {

#define START_BUFFER_DURATION_DEFAULT (1000*1000)
#define HIGH_BUFFERING_LEVEL_DEFAULT (5000*1000)
#define MAX_BUFFER_DURATION_DEFAULT (40*1000*1000)

    player_type_set::player_type_set()
    {
        reset();
    }

    player_type_set::~player_type_set() = default;

    void player_type_set::reset()
    {
        startBufferDuration = START_BUFFER_DURATION_DEFAULT;
        highLevelBufferDuration = HIGH_BUFFERING_LEVEL_DEFAULT;
        maxBufferDuration = MAX_BUFFER_DURATION_DEFAULT;
        url = "";
        refer = "";
        timeout_ms = 15000;
        RTMaxDelayTime = 0;
        bLooping = false;
        bDisableAudio = false;
        bDisableVideo = false;
        bMute = false;
        bDisableBufferManager = false;
        bLowLatency = false;
        mView = nullptr;
        mAutoSwitchTime = INT64_MIN;
        mVolume = 1.0;
        mPlayerListener = {nullptr,};
        rate = 1.0;
        http_proxy = "";
        userAgent = "";
        clearShowWhenStop = false;
        bEnableTunnelRender = false;
        bEnableHwVideoDecode = true;
        // don't need reset for each playback
//        bEnableVRC = false;
#if TARGET_OS_IPHONE
        lowMemSize = 50 * 1024 * 1024;// 50M
#else
        lowMemSize = 100 * 1024 * 1024; //100M
#endif
        mDefaultBandWidth = 0;
        mVideoBackgroundColor = 0xFF000000;
        // Don't reset AnalyticsID
        mOptions.reset();
        maxASeekDelta = 21 * 1000 * 1000;
        maxVideoRecoverSize = 300;
    }
}

