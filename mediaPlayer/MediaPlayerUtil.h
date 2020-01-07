//
//  MediaPlayerUtil.cpp
//  ApsaraPlayer
//
//  Created by huang_jiafa on 2010/01/30.
//  Copyright (c) 2019 Aliyun. All rights reserved.
//

#ifndef ApsaraPlayerUtil_h
#define ApsaraPlayerUtil_h

#include <deque>
#include <string>
#include "demuxer/demuxer_service.h"
#include "utils/AFMediaType.h"
#include "native_cicada_player_def.h"
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
            readEvent_Loop

        };

        MediaPlayerUtil() = default;

        ~MediaPlayerUtil() = default;

        void notifyPlayerLoop(int64_t time);

        void notifyRead(enum readEvent event);

        void render(int64_t pts);

        void reset();

        float getVideoRenderFps()
        { return mVideoRenderFps; }

        static void getPropertyJSONStr(const std::string &name, CicadaJSONArray &array, bool isArray,
                                       std::deque<StreamInfo *> &streamInfoQueue, demuxer_service *service);

        static void addPropertyType(CicadaJSONItem &item, StreamType type);

        static void addURLProperty(const std::string &name, CicadaJSONArray &array, IDataSource *dataSource);

    private:
        int mTotalRenderCount = 0;
        int mLastRenderCount = 0;
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

        float mVideoRenderFps = 0;
    };
}

#endif /* AlivcPlayer_h */
