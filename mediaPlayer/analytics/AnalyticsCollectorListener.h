//
//  AnalyticsCollectorListener.h
//  Analytics
//
//  Created by huang_jiafa on 2018/11/07.
//  Copyright (c) 2018 Aliyun. All rights reserved.
//

#ifndef ANALYTICS_COLLECTOR_LISTENER_H
#define ANALYTICS_COLLECTOR_LISTENER_H

#include <string>
#include "native_cicada_player_def.h"
#include "AnalyticsType.h"

namespace Cicada {
    class AnalyticsCollectorListener {
    public:

        virtual void OnPlayerUpdateSessionId(const std::string& sessionId) {};

        virtual void OnSetLooping(bool looping) {};

        virtual void OnSetRenderMirrorMode(MirrorMode mode) {};

        virtual void OnRotate(RotateMode mode) {};

        virtual void OnPlay() {};

        virtual void OnPause() {};

        virtual void OnResume(int useTimeMS) {};

        virtual void OnStop() {};

        virtual void OnSeekStart(int64_t toTimeStampMs, SeekMode mode) {};

        virtual void OnSeekEnd(int useTimeMS) {};

        // player status
        virtual void OnEOS() {};

        virtual void OnReplayEvent(bool success) {};

        virtual void OnStatusChanged(PlayerStatus newStatus) {};

        virtual void
        OnError(int errCode, const std::string &errMsg, const std::string &requestID) {};

        virtual void OnFirstRender() {};

        virtual void OnFirstVideoRender() {};

        virtual void OnFirstAudioRender() {};

        virtual void OnVideoSizeChanged(int width, int height) {};

        virtual void OnSnapshot() {};

        // source
        virtual void OnPrepare() {};

        virtual void OnPrepared() {};

        virtual void OnCompletion() {};
        
        virtual void OnSetThumbnailURL(const std::string &URL) {};

        virtual void OnThumbnailReady(bool success, int useTimeMS) {};

        virtual void OnCurrentBitrate(int bitrate) {};
        
        // status
        virtual void OnLoadingStart() {};

        virtual void OnLoadingEnd(int useTimeMS) {};

        virtual void OnLoopingStart() {};

        //other
        virtual void OnSwitchToSoftDecode() {};

        virtual void OnBlackInfo() {};
    };

}// namespace Cicada

#endif // ANALYTICS_COLLECTOR_LISTENER_H

