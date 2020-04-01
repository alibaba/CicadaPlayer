//
//  MediaPlayerConfig.h
//  apsara_player
//
//  Created by shiping.csp on 2018/11/12.
//

#ifndef MediaPlayerConfig_h
#define MediaPlayerConfig_h

#include <stdio.h>
#include <string>
#include <vector>
#include <utils/CicadaType.h>

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
        /* enable tunnel rander*/
        bool bEnableTunnelRender;
        std::vector<std::string> customHeaders;

    public:
        std::string toString() const;
    };
}


#endif /* MediaPlayerConfig_h */
