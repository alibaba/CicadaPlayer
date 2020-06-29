//
// Created by moqi on 2018/4/28.
//

#ifndef FRAMEWORK_HLSSTREAM_H
#define FRAMEWORK_HLSSTREAM_H

#include <demuxer/play_list/segment_decrypt/ISegDecrypter.h>
#include <demuxer/sample_decrypt/HLSSampleAesDecrypter.h>
#include <utils/afThread.h>
#include <utils/CicadaJSON.h>
#include "AbstractStream.h"
#include "SegmentTracker.h"
#include "../demuxer_service.h"
#include <condition_variable>
#include <deque>
#include <mutex>
#include <map>
#include <atomic>

namespace Cicada {
    class HLSStream : public AbstractStream {

    public:
        HLSStream(SegmentTracker *pTracker, int id);

        ~HLSStream() override;

        int getStreamType();

        int GetRemainSegmentCount();

        int open() override;

        void close() override;

        int getId()
        {
            return mId;
        }

        int read(unique_ptr<IAFPacket> &packet) override;

        int GetNbStreams() override;

        int GetStreamMeta(Stream_meta *meta, int index, bool sub) override;

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

        int getNBStream() override;

        void interrupt(int inter) override;

        std::string GetProperty(const string &key);


    private:

        static int read_callback(void *arg, uint8_t *buffer, int size);

        static int Decrypter_read_callback(void *arg, uint8_t *buffer, int size);

//        static int64_t seek_callback(void *arg, int64_t offset, int whence);


        int read_internal(std::unique_ptr<IAFPacket> &packet);

        int64_t seek_internal(uint64_t segNum, int64_t us);

        int open_internal();

        void resetSource();

        void recreateSource(const string &url);

        void clearDataFrames();

        int updateSegDecrypter();

        bool updateKey();

        int updateSampleAesDecrypter();

        int updateDecrypter();

        void interrupt_internal(int inter);

        CicadaJSONArray openInfoArray;

        int openSegment(const string &uri);

        bool updateIV() const;

        enum OpenType {
            SegNum,SegPosition
        };

        int reopenSegment(uint64_t num, OpenType openType);
    private:

        enum dataSourceStatus {
            dataSource_status_valid,
            dataSource_status_invalid,
            dataSource_status_error
        };
        SegmentTracker *mPTracker = nullptr;
        int mId = -1;
        demuxer_service *mPDemuxer = nullptr;
        IDataSource *mPdataSource = nullptr;
        atomic_bool mIsOpened{false};
        bool mIsEOS = false; //demuxer eos
        bool mIsDataEOS = false;
        bool mReopen = false;
        bool mSwitchNeedBreak = false;
        std::shared_ptr<segment> mCurSeg = nullptr;
        std::atomic_bool mStopOnSegEnd{false};
        bool mLastReadSuccess{false};
        std::mutex mDataMutex;
        std::condition_variable mWaitCond;
        std::deque<unique_ptr<IAFPacket>> mQueue;
        IDataSource *mSegKeySource = nullptr;
        std::mutex mHLSMutex;

        int read_thread();

        std::atomic_int mError{0};
        dataSourceStatus mDataSourceStatus = dataSource_status_invalid;
        int mDataSourceError = 0;
        int64_t mSeekPendingUs = -1;
        bool mIsOpened_internal = false;
        std::atomic_bool mInterrupted{false};
        afThread *mThreadPtr = nullptr;
        std::unique_ptr<ISegDecrypter> mSegDecrypter = nullptr;
        std::unique_ptr<HLSSampleAesDecrypter> mSampeAesDecrypter = nullptr;
        string mKeyUrl = "";
        uint8_t mKey[16];


        struct segmentTimeInfo {
            bool seamlessPoint = false;
            int64_t timePosition = INT64_MIN;
            int64_t time2ptsDelta = INT64_MIN;
            int64_t frameDuration = INT64_MIN;
            int64_t lastFramePts = INT64_MIN;
        };

        std::map<int, segmentTimeInfo> mStreamStartTimeMap;
        bool mProtectedBuffer{false};

        int64_t mLiveStartIndex{-3};//segment index to start live streams at (negative values are from the end)
    };
}


#endif //FRAMEWORK_HLSSTREAM_H
