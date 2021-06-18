//
// Created by yuyuan on 2021/03/17.
//

#ifndef DEMUXER_DASH_DASH_STREAM_H
#define DEMUXER_DASH_DASH_STREAM_H

#include "demuxer/DemuxerMetaInfo.h"
#include "demuxer/demuxer_service.h"
#include "demuxer/play_list/AbstractStream.h"
#include "utils/CicadaJSON.h"
#include "utils/afThread.h"
#include <atomic>
#include <condition_variable>
#include <deque>
#include <map>
#include <mutex>

namespace Cicada {

    class DashSegmentTracker;
    namespace Dash {
        class DashSegment;
    }

    class DashStream : public AbstractStream {

    public:
        DashStream(DashSegmentTracker *pTracker, int id);

        ~DashStream() override;

        int getStreamType() const;

        int GetRemainSegmentCount();

        int open() override;

        void close() override;

        int getId() const
        {
            return mId;
        }

        int read(unique_ptr<IAFPacket> &packet) override;

        int GetNbStreams() const override;

        int GetStreamMeta(Stream_meta *meta, int index, bool sub) const override;

        bool isOpened() override;

        int start() override;

        int stop() override;

        int64_t seek(int64_t us, int flags) override;

        uint64_t getCurSegNum() override;


        int stopOnSegEnd(bool stop) override;

        int SetCurSegNum(uint64_t num) override;

        uint64_t getCurSegPosition() override;

        int setCurSegPosition(uint64_t position) override;

        bool isLive() override;

        int64_t getDuration() override;

        int getNBStream() const override;

        void interrupt(int inter) override;

        std::string GetProperty(const string &key);
        
        bool isRealTimeStream();

        int UpdateInitSection();

        int64_t getDurationToStartStream();

    private:

        static int read_callback(void *arg, uint8_t *buffer, int size);
        static int64_t seek_callback(void *arg, int64_t offset, int whence);


        int read_internal(std::unique_ptr<IAFPacket> &packet);

        int64_t seek_internal(uint64_t segNum, int64_t us);

        int open_internal();

        void resetSource();

        void recreateSource(const string &url);

        void clearDataFrames();

        void interrupt_internal(int inter);

        CicadaJSONArray openInfoArray;

        int openSegment(const string &uri, int64_t start = INT64_MIN, int64_t end = INT64_MIN);

        int tryOpenSegment(const string &uri, int64_t start, int64_t end);

        int64_t getPackedStreamPTS();

        int createDemuxer();

        int readSegment(const uint8_t *buffer, int size);

        int64_t seekSegment(off_t offset, int whence);

        int updateSegment();

        enum OpenType {
            SegNum, SegPosition
        };

        int reopenSegment(uint64_t num, OpenType openType);

    private:

        enum dataSourceStatus {
            dataSource_status_valid,
            dataSource_status_invalid,
            dataSource_status_error
        };
        DashSegmentTracker *mPTracker = nullptr;
        int mId = -1;
        std::unique_ptr<demuxer_service> mPDemuxer = nullptr;
        IDataSource *mPdataSource = nullptr;
        atomic_bool mIsOpened{false};
        atomic_bool mIsEOS{false}; //demuxer eos
        bool mIsDataEOS = false;
        bool mReopen = false;
        atomic_bool mSwitchNeedBreak{false};
        Dash::DashSegment *mCurSeg = nullptr;
        Dash::DashSegment *mCurInitSeg{nullptr};
        uint8_t *mInitSegBuffer{nullptr};
        int64_t mInitSegSize{0};
        int mInitSegPtr{0};
        std::atomic_bool mStopOnSegEnd{false};
        bool mLastReadSuccess{false};
        std::mutex mDataMutex;
        std::condition_variable mWaitCond;
        std::deque<unique_ptr<IAFPacket>> mQueue;
        IDataSource *mSegKeySource = nullptr;
        mutable std::mutex mHLSMutex;

        int read_thread();

        std::atomic_int mError{0};
        int mDataSourceError = 0;
        int64_t mSeekPendingUs = -1;
        std::atomic<bool> mIsOpened_internal{false};
        std::atomic_bool mInterrupted{false};
        afThread *mThreadPtr = nullptr;
        string mKeyUrl = "";
        uint8_t mKey[16];


        struct segmentTimeInfo {
            bool seamlessPoint = false;
            int64_t timePosition = INT64_MIN;
            int64_t time2ptsDelta = INT64_MIN;
            int64_t utcTime = INT64_MIN;
            int64_t utc2ptsDelta = INT64_MIN;
            int64_t frameDuration = INT64_MIN;
            int64_t lastFramePts = INT64_MIN;
        };

        std::map<int, segmentTimeInfo> mStreamStartTimeMap;

        int64_t mPacketFirstPts = INT64_MAX;

        std::unique_ptr<DemuxerMetaInfo> mDemuxerMeta = nullptr;
        int OpenedStreamIndex = 0;
        bool mProtectedBuffer{false};

        int64_t mStreamStartTime = 0;
        int64_t mSuggestedPresentationDelay = 0;

    };
}


#endif //DEMUXER_DASH_DASH_STREAM_H
