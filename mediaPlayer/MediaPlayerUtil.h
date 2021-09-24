//
//  MediaPlayerUtil.cpp
//  ApsaraPlayer
//
//  Created by huang_jiafa on 2010/01/30.
//  Copyright (c) 2019 Aliyun. All rights reserved.
//

#ifndef ApsaraPlayerUtil_h
#define ApsaraPlayerUtil_h

#include "demuxer/demuxer_service.h"
#include "native_cicada_player_def.h"
#include "utils/AFMediaType.h"
#include <deque>
#include <string>
//#include "render_engine/math/geometry.h"

using namespace Cicada;

class CicadaJSONItem;

class CicadaJSONArray;

namespace Cicada {
    class MediaPlayerUtil {
    public:
        enum readEvent {
            readEvent_Again,
            readEvent_Got,
            readEvent_timeOut,
            readEvent_Loop,
            readEvent_Network,

        };

        MediaPlayerUtil() = default;

        ~MediaPlayerUtil() = default;

        void notifyPlayerLoop(int64_t time);

        void notifyRead(enum readEvent event, uint64_t size);

        void videoRendered(bool rendered);

        void reset();

        void getVideoDroppedInfo(uint64_t &total, uint64_t &dropped)
        {
            total = mTotalRenderCount;
            dropped = mDroppedRenderCount;
        }

        float getVideoRenderFps()
        {
            return mVideoRenderFps;
        }

        float getCurrentDownloadSpeed() const
        {
            return mCurrentDownloadSpeed;
        }

        void updateBufferInfo(const CicadaJSONItem &info);

        std::map<int64_t, int64_t> getNetworkSpeed(int64_t timeFrom, int64_t timeTo);

        std::map<int64_t, std::string> getBufferInfo(int64_t timeFrom, int64_t timeTo);

        static void getPropertyJSONStr(const std::string &name, CicadaJSONArray &array, bool isArray,
                                       std::deque<StreamInfo *> &streamInfoQueue, demuxer_service *service);

        static void addPropertyType(CicadaJSONItem &item, StreamType type);

        static void addURLProperty(const std::string &name, CicadaJSONArray &array, IDataSource *dataSource);

    private:
        std::atomic<uint64_t> mTotalRenderCount{0};
        std::atomic<uint64_t> mDroppedRenderCount{0};
        uint64_t mLastRenderCount = 0;
        int64_t mFirstRenderTime = 0;
        int64_t mLastRenderTime = 0;

        int64_t mLastLoopTime = 0;
        int64_t mLoopIndex = 0;
        int64_t mReadIndex = 0;
        int64_t mReadLoopIndex = 0;
        int64_t mReadAgainIndex = 0;
        int64_t mReadGotIndex = 0;
        int64_t mReadTimeOutIndex = 0;
        int64_t mLastReadTime = 0;
        std::atomic<uint64_t> mReadGotSize{0};
        float mCurrentDownloadSpeed{0};
        float mVideoRenderFps = 0;

        std::mutex utilMutex{};
        const int MAX_COUNT = 600;
        std::map<int64_t, int64_t> mNetworkSpeed{};
        std::map<int64_t, std::string> mBufferInfo{};
    };
}// namespace Cicada

#endif /* ApsaraPlayerUtil_h */
