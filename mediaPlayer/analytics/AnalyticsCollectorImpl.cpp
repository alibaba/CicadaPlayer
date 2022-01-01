//
//  AnalyticsCollectorImpl.cpp
//  Analytics
//
//  Created by huang_jiafa on 2018/11/06.
//  Copyright (c) 2018 Aliyun. All rights reserved.
//

#include "AnalyticsCollectorImpl.h"
#include "AnalyticsUtils.h"

namespace Cicada {
    AnalyticsCollectorImpl::AnalyticsCollectorImpl(AnalyticsQueryListener *queryListener)
        : IAnalyticsCollector(queryListener)
    {
    }

    AnalyticsCollectorImpl::~AnalyticsCollectorImpl()
    {
    }

    void AnalyticsCollectorImpl::AddListener(AnalyticsCollectorListener *listener)
    {
        mListener.remove(listener);
        mListener.push_back(listener);
    }

    void AnalyticsCollectorImpl::RemoveListener(AnalyticsCollectorListener *listener)
    {
        mListener.remove(listener);
    }

    void AnalyticsCollectorImpl::ReportUpdatePlaySession(const std::string &sessionId)
    {
        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnPlayerUpdateSessionId(sessionId);
            }
        }
    }

    // setting
    void AnalyticsCollectorImpl::ReportLooping(bool looping)
    {
        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnSetLooping(looping);
            }
        }
    }

    void AnalyticsCollectorImpl::ReportRenderMirrorMode(MirrorMode mode)
    {
        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnSetRenderMirrorMode(mode);
            }
        }
    }

    void AnalyticsCollectorImpl::ReportRotate(RotateMode mode)
    {
        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnRotate(mode);
            }
        }
    }

    // operation
    void AnalyticsCollectorImpl::ReportPlay()
    {
        if ((PLAYER_PREPARED != mData.status)
                && (PLAYER_PAUSED != mData.status)
                && (PLAYER_STOPPED != mData.status)
                && (PLAYER_COMPLETION != mData.status)
           ) {
            return;
        }

        if ((PLAYER_PAUSED == mData.status) && (0 < mPauseTimeMS)) {
            int useTime = static_cast<int>(AnalyticsUtils::getTimeMS() - mPauseTimeMS);

            for (AnalyticsCollectorListener *iter : mListener) {
                if (nullptr != iter) {
                    iter->OnResume(useTime);
                }
            }

            mPauseTimeMS = 0;
        } else {
            mStartTimeMS = AnalyticsUtils::getTimeMS();

            for (AnalyticsCollectorListener *iter : mListener) {
                if (nullptr != iter) {
                    iter->OnPlay();
                }
            }
        }
    }

    void AnalyticsCollectorImpl::ReportPause()
    {
        mPauseTimeMS = AnalyticsUtils::getTimeMS();

        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnPause();
            }
        }
    }

    void AnalyticsCollectorImpl::ReportSeekStart(int64_t toTimeStampMs, SeekMode mode)
    {
        mSeekTimeMS = AnalyticsUtils::getTimeMS();

        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnSeekStart(toTimeStampMs, mode);
            }
        }
    }

    void AnalyticsCollectorImpl::ReportSeekEnd()
    {
        if (0 >= mSeekTimeMS) {
            return;
        }

        int useTime = static_cast<int>(AnalyticsUtils::getTimeMS() - mSeekTimeMS);
        mSeekTimeMS = 0;

        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnSeekEnd(useTime);
            }
        }
    }

    void AnalyticsCollectorImpl::ReportStop()
    {
        if (PLAYER_STOPPED == mData.status) {
            return;
        }

        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnStop();
            }
        }
    }

    // player status
    void AnalyticsCollectorImpl::ReportEOS()
    {
        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnEOS();
            }
        }
    }

    void AnalyticsCollectorImpl::ReportReplayEvent(bool success)
    {
        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnReplayEvent(success);
            }
        }
    }

    void AnalyticsCollectorImpl::ReportPlayerStatueChange(PlayerStatus oldStatus,
            PlayerStatus newStatus)
    {
        mData.status = newStatus;

        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnStatusChanged(newStatus);
            }
        }
    }

    void AnalyticsCollectorImpl::ReportError(int errCode, const std::string &errMsg,
            const std::string &requestID)
    {
        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnError(errCode, errMsg, requestID);
            }
        }
    }

    void AnalyticsCollectorImpl::ReportFirstRender()
    {
        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnFirstRender();
            }
        }
    }

    void AnalyticsCollectorImpl::ReportFirstVideoRender()
    {
        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnFirstVideoRender();
            }
        }
    }

    void AnalyticsCollectorImpl::ReportFirstAudioRender()
    {
        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnFirstAudioRender();
            }
        }
    }

    void AnalyticsCollectorImpl::ReportVideoSizeChanged(int width, int height)
    {
        mData.width = width;
        mData.height = height;

        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnVideoSizeChanged(width, height);
            }
        }
    }

    void AnalyticsCollectorImpl::ReportSnapshot()
    {
        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnSnapshot();
            }
        }
    }

    // IAnalyticSource
    // source
    void AnalyticsCollectorImpl::ReportPrepare()
    {
        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnPrepare();
            }
        }
    }

    void AnalyticsCollectorImpl::ReportCompletion()
    {
        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnCompletion();
            }
        }
    }

    // status
    void AnalyticsCollectorImpl::ReportLoadingStart()
    {
        mLoadingStartTimeMs = AnalyticsUtils::getTimeMS();

        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnLoadingStart();
            }
        }
    }

    void AnalyticsCollectorImpl::ReportLoadingEnd()
    {
        int useTime = static_cast<int>(AnalyticsUtils::getTimeMS() - mLoadingStartTimeMs);

        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnLoadingEnd(useTime);
            }
        }
    }

    void AnalyticsCollectorImpl::ReportLoopingStart()
    {
        mLoadingStartTimeMs = AnalyticsUtils::getTimeMS();

        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnLoopingStart();
            }
        }
    }

    void AnalyticsCollectorImpl::ReportPrepared()
    {
        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnPrepared();
            }
        }
    }

    void AnalyticsCollectorImpl::ReportThumbnailURL(const std::string &URL)
    {
        mSetThumbnailTimeMs = AnalyticsUtils::getTimeMS();

        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnSetThumbnailURL(URL);
            }
        }
    }

    void AnalyticsCollectorImpl::ReportThumbnailReady(bool success)
    {
        int useTime = static_cast<int>(AnalyticsUtils::getTimeMS() - mSetThumbnailTimeMs);

        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnThumbnailReady(success, useTime);
            }
        }
    }

    void AnalyticsCollectorImpl::ReportCurrentBitrate(int bitrate)
    {
        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnCurrentBitrate(bitrate);
            }
        }
    }



    void AnalyticsCollectorImpl::ReportSwitchToSoftDecode()
    {
        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnSwitchToSoftDecode();
            }
        }
    }

    void AnalyticsCollectorImpl::ReportBlackInfo()
    {
        for (AnalyticsCollectorListener *iter : mListener) {
            if (nullptr != iter) {
                iter->OnBlackInfo();
            }
        }
    }
}


