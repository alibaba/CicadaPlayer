//
//  AnalyticsCollectorImpl.h
//  Analytics
//
//  Created by huang_jiafa on 2018/11/06.
//  Copyright (c) 2018 Aliyun. All rights reserved.
//

#ifndef ANALYTICS_COLLECTOR_IMPL
#define ANALYTICS_COLLECTOR_IMPL

#include <mutex>
#include <list>
#include "IAnalyticsCollector.h"
#include <atomic>

namespace Cicada {
    // store variables which may need provide by getAnalyticsData()
    class CollectorData : public ICollectorData {
    public:
        int startupUseTimeMS = 0;
        int prepareUseTimeMS = 0;

        int width = 0;
        int height = 0;
        PlayerStatus status = PLAYER_IDLE;
    };

    class AnalyticsCollectorImpl : public IAnalyticsCollector {
    public:
        AnalyticsCollectorImpl(AnalyticsQueryListener *queryListener);

        virtual ~AnalyticsCollectorImpl();

        void AddListener(AnalyticsCollectorListener *listener) override;

        void RemoveListener(AnalyticsCollectorListener *listener) override;

        const ICollectorData *getAnalyticsData() override
        { return &mData; };

        // IAnalyticPlayer
        // setting
        void ReportLooping(bool looping) override;

        void ReportRenderMirrorMode(MirrorMode mode) override;

        void ReportRotate(RotateMode mode) override;

        // operation
        void ReportPlay() override;

        void ReportPause() override;

        void ReportSeekStart(int64_t toTimeStampMs, SeekMode mode) override;

        void ReportSeekEnd() override;

        void ReportStop() override;

        // player status
        void ReportEOS() override;

        void ReportReplayEvent(bool success) override;

        void ReportPlayerStatueChange(PlayerStatus oldStatus, PlayerStatus newStatus) override;

        void ReportError(int errCode, const std::string &errMsg, const std::string &requestID) override;

        // IAnalyticRender
        void ReportFirstRender() override;

        void ReportFirstVideoRender() override;

        void ReportFirstAudioRender() override;

        void ReportVideoSizeChanged(int width, int height) override;

        void ReportSnapshot() override;

        // IAnalyticSource
        // source
        void ReportPrepare() override;

        void ReportPrepared() override;

        void ReportCompletion() override;

        // status
        void ReportLoadingStart() override;

        void ReportLoadingEnd() override;

        void ReportLoopingStart() override;


        // Track & selection        
        void ReportThumbnailURL(const std::string &URL) override;

        void ReportThumbnailReady(bool success) override;

        void ReportCurrentBitrate(int bitrate) override;

        //other

        void ReportBlackInfo() override;

        void ReportSwitchToSoftDecode() override;

    protected:
        std::atomic <int64_t> mStartTimeMS {0};
        int64_t mPauseTimeMS = 0;
        std::atomic<int64_t> mSeekTimeMS{0};
        int64_t mGetURLStartTimeMs = 0;
        int64_t mLoadingStartTimeMs = 0;
        int64_t mSetThumbnailTimeMs = 0;

        CollectorData mData;

        // don't edit list when using
        std::list<AnalyticsCollectorListener *> mListener;
    };
}
#endif
