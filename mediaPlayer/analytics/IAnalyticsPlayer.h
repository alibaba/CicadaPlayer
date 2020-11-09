//
//  IAnalyticPlayer.h
//  Analytics
//
//  Created by huang_jiafa on 2018/11/07.
//  Copyright (c) 2018 Aliyun. All rights reserved.
//

#ifndef IANALYTICS_PLAYER_H
#define IANALYTICS_PLAYER_H

#include <string>
#include "AnalyticsType.h"
#include "native_cicada_player_def.h"

namespace Cicada {

    class IAnalyticPlayer {
    public:
        virtual ~IAnalyticPlayer() = default;

        // setting
        virtual void ReportLooping(bool looping) = 0;

        virtual void ReportRenderMirrorMode(MirrorMode mode) = 0;

        virtual void ReportRotate(RotateMode mode) = 0;

        // operation
        virtual void ReportPlay() = 0;

        virtual void ReportPause() = 0;

        virtual void ReportStop() = 0;

        virtual void ReportSeekStart(int64_t toTimeStampMs, SeekMode mode) = 0;

        virtual void ReportSeekEnd() = 0;

        // player status
        virtual void ReportEOS() = 0; // TODO
        virtual void ReportReplayEvent(bool success) = 0; // TODO
        virtual void ReportPlayerStatueChange(PlayerStatus oldStatus, PlayerStatus newStatus) = 0;

        virtual void
        ReportError(int errCode, const std::string &errMsg, const std::string &requestID) = 0;

        virtual void ReportUpdatePlaySession(const std::string &sessionId) = 0;

    };

}// namespace Cicada
#endif // IANALYTICS_PLAYER_H


