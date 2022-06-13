//
//  MediaPlayerConfig.h
//  apsara_player
//
//  Created by shiping.csp on 2018/11/12.
//

#ifndef MediaPlayerConfig_h
#define MediaPlayerConfig_h

#include <cstdio>
#include <string>
#include <utils/CicadaType.h>
#include <vector>

namespace Cicada {

    class CICADA_CPLUS_EXTERN MediaPlayerConfig {
    public:
        MediaPlayerConfig();

    public:
        std::string referer;
        std::string httpProxy;
        std::string userAgent;
        int networkTimeout;
        int networkRetryCount;
        /*for live*/
        int maxDelayTime;
        int maxBufferDuration;
        int highBufferDuration;
        int startBufferDuration;
        /* true is to clear image show when stop */
        bool bClearShowWhenStop;
        /* enable tunnel render*/
        bool bEnableTunnelRender;
        std::vector<std::string> customHeaders;
        /* set the video format for renderFrame callback
         * vtb decoder only, equal to OSType, not be supported by other decoder
         * support 420v 420f y420 BGRA
         * */
        uint32_t pixelBufferOutputFormat;

        int liveStartIndex;

        bool mDisableAudio;
        bool mDisableVideo;

        int mPositionTimerIntervalMs;
        uint64_t mMaxBackwardBufferDuration;
        int maxBufferMemoryKB;
        std::string localCacheDir;
        bool preferAudio;

    public:
        std::string toString() const;
    };
}


#endif /* MediaPlayerConfig_h */
