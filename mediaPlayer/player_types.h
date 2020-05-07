//
// Created by moqi on 2018/9/21.
//

#ifndef CICADA_PLAYER_PLAYER_TYPES_H
#define CICADA_PLAYER_PLAYER_TYPES_H


#include <cstdint>
#include <string>
#include <vector>
#include "ICicadaPlayer.h"
#include "playerOptions.h"
#include <atomic>

namespace Cicada {

    class player_type_set {
    public:
        player_type_set();

        ~player_type_set();

        void reset();

    public:
        int64_t startBufferDuration = 0;
        int64_t highLevelBufferDuration = 0;
        int64_t maxBufferDuration = 0;
        uint64_t lowMemSize = 0;
        std::string url{""};
        std::string refer{""};
        std::string userAgent{""};
        int timeout_ms{15000};
        int RTMaxDelayTime{0};
        bool bLooping{false};
        bool bDisableAudio{false};
        bool bDisableVideo{false};
        bool bMute = false;
        bool bDisableBufferManager = false;
        bool bLowLatency = false;

        std::atomic<void *> mView{nullptr};
        ScaleMode scaleMode = ScaleMode::SM_FIT;
        RotateMode rotateMode = RotateMode::ROTATE_MODE_0;
        MirrorMode mirrorMode = MirrorMode::MIRROR_MODE_NONE;
        int64_t mAutoSwitchTime{INT64_MIN};
        atomic<float> mVolume{1.0};
        playerListener mPlayerListener;
        atomic<float> rate {1.0};
        std::string http_proxy = "";
        std::vector<std::string> customHeaders;
        bool clearShowWhenStop = false;
        bool bEnableTunnelRender = true;
        bool bEnableHwVideoDecode = true;
        int64_t AnalyticsID = -1;
        int mDefaultBandWidth = 0;
        playerOptions mOptions;
        uint32_t mVideoBackgroundColor = 0xFF000000;
        bool bEnableVRC = false;
        int maxASeekDelta = 21 * 1000 * 1000;//us

        int maxVideoRecoverSize;
    };
}


#endif //CICADA_PLAYER_PLAYER_TYPES_H
