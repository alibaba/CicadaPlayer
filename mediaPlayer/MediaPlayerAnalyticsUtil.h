//
// Created by SuperMan on 2021/10/28.
//

#ifndef QALIYUNPLAYERTEST_MEDIAPLAYERANALYTICSUTIL_H
#define QALIYUNPLAYERTEST_MEDIAPLAYERANALYTICSUTIL_H

#include <list>
#include <map>
#include <mutex>
#include <utils/CicadaJSON.h>

#include "demuxer/demuxer_service.h"
#include "native_cicada_player_def.h"
#include "utils/AFMediaType.h"

namespace Cicada {
    class RenderInfo {

    public:
        RenderInfo();

        ~RenderInfo();

        void reset();

        void videoRendered(bool rendered);

        std::string toString();

    public:
        int JANK_GAP = 200;
        int MAX_GRADE_NUM = 5;//0~200,...,800~1s,1s~
        int totalCount{0};
        int droppedCount{0};
        std::map<int, int> jankTimesMap{};
        std::map<int, int64_t> jankCostMap{};

    private:
        int64_t lastRenderedTimeMs{INT64_MIN};
    };

    class MediaPlayerAnalyticsUtil {
    public:
        MediaPlayerAnalyticsUtil();

        ~MediaPlayerAnalyticsUtil();

        void reset();

        std::map<int64_t, int64_t> getNetworkSpeed(int64_t timeFrom, int64_t timeTo);

        std::map<int64_t, std::string> getBufferInfo(int64_t timeFrom, int64_t timeTo);

        std::string getNetworkRequestInfos(int64_t timeFrom, int64_t timeTo);

        std::string getRenderInfoAndReset();

        void updateNetworkReadSize(uint64_t size);

        void updateBufferInfo(bool force, int64_t videoDuration, int64_t audioDuration);

        void notifyNetworkEvent(const std::string &url, const CicadaJSONItem &eventParam);

        void videoRendered(bool rendered);

    public:
        static void getPropertyJSONStr(const std::string &name, CicadaJSONArray &array, bool isArray,
                                       std::deque<StreamInfo *> &streamInfoQueue, demuxer_service *service);

        static void addPropertyType(CicadaJSONItem &item, StreamType type);

        static void addURLProperty(const std::string &name, CicadaJSONArray &array, IDataSource *dataSource);

    private:
        static void filterNetworkInfo(CicadaJSONArray &info, int64_t timeFrom, int64_t timeTo,
                                      const std::function<void(CicadaJSONItem &event)> &callback);

    private:
        std::mutex utilMutex{};

        const int MAX_COUNT = 600;
        std::map<int64_t, float> mNetworkSpeed{};
        std::map<int64_t, std::string> mBufferInfo{};

        std::list<std::string> mNetworkUrls{};
        std::map<std::string, std::string> mNetworkInfos{};

        int64_t updateBufferInfoLastTimeMs{INT64_MIN};

        uint64_t mLastReadTime{0};
        std::atomic<uint64_t> mReadGotSize{0};

        std::mutex renderInfoMutex{};
        RenderInfo mRenderInfo{};
    };
}// namespace Cicada

#endif//QALIYUNPLAYERTEST_MEDIAPLAYERANALYTICSUTIL_H
