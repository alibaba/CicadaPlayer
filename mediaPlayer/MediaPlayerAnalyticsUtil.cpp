//
// Created by SuperMan on 2021/10/28.
//

#include "MediaPlayerAnalyticsUtil.h"
#include <utils/timer.h>

using namespace Cicada;


MediaPlayerAnalyticsUtil::MediaPlayerAnalyticsUtil()
{}

MediaPlayerAnalyticsUtil::~MediaPlayerAnalyticsUtil()
{
    reset();
}

void MediaPlayerAnalyticsUtil::reset()
{
    mNetworkSpeed.clear();
    mBufferInfo.clear();
    mNetworkUrls.clear();
    mNetworkInfos.clear();

    updateBufferInfoLastTimeMs = INT64_MIN;
    mLastReadTime = 0;
    mReadGotSize = 0;
}

void MediaPlayerAnalyticsUtil::updateNetworkReadSize(uint64_t size)
{
    mReadGotSize += size;

    int64_t time = af_gettime_relative();

    if (mLastReadTime == 0) {
        mLastReadTime = time;
    } else {
        float timeS = float(time - mLastReadTime) / 1000000;

        if (timeS > 1.0) {
            float speed = (float) mReadGotSize * 8 / timeS;

            {
                std::lock_guard<std::mutex> lockGuard(utilMutex);
                if (mNetworkSpeed.size() >= MAX_COUNT) {
                    mNetworkSpeed.erase(mNetworkSpeed.begin());
                }
                mNetworkSpeed[time] = speed;
            }

            mLastReadTime = time;
            mReadGotSize = 0;
        }
    }
}

std::map<int64_t, int64_t> MediaPlayerAnalyticsUtil::getNetworkSpeed(int64_t timeFrom, int64_t timeTo)
{

    std::map<int64_t, int64_t> result{};
    {
        std::lock_guard<std::mutex> lockGuard(utilMutex);
        for (auto &item : mNetworkSpeed) {
            if (item.first >= timeFrom && item.first < timeTo) {
                result.insert(item);
            }
        }
    }

    return result;
}

std::map<int64_t, std::string> MediaPlayerAnalyticsUtil::getBufferInfo(int64_t timeFrom, int64_t timeTo)
{
    std::map<int64_t, std::string> result{};
    {
        std::lock_guard<std::mutex> lockGuard(utilMutex);
        for (auto &item : mBufferInfo) {
            if (item.first >= timeFrom && item.first < timeTo) {
                result.insert(item);
            }
        }
    }
    return result;
}

void MediaPlayerAnalyticsUtil::notifyNetworkEvent(const std::string &url, const CicadaJSONItem &eventParam)
{

    std::lock_guard<std::mutex> lockGuard(utilMutex);

    if (mNetworkInfos.count(url) > 0) {
        CicadaJSONArray infos(mNetworkInfos[url]);
        infos.addJSON(eventParam);
        mNetworkInfos[url] = infos.printJSON();
    } else {
        CicadaJSONArray infos{};
        infos.addJSON(eventParam);
        mNetworkInfos[url] = infos.printJSON();
        mNetworkUrls.push_back(url);
    }

    if (mNetworkUrls.size() > 3) {
        std::string targetUrl = mNetworkUrls.front();
        mNetworkUrls.erase(mNetworkUrls.begin());
        mNetworkInfos.erase(targetUrl);
    }
}

std::string MediaPlayerAnalyticsUtil::getNetworkRequestInfos(int64_t timeFrom, int64_t timeTo)
{

    CicadaJSONArray requestInfoArray{};
    std::lock_guard<std::mutex> lockGuard(utilMutex);
    for (auto &item : mNetworkUrls) {
        const std::string &url = item;
        CicadaJSONArray wantedInfos{};
        CicadaJSONArray infos(mNetworkInfos[url]);
        filterNetworkInfo(infos, timeFrom, timeTo, [&wantedInfos](CicadaJSONItem &event) -> void { wantedInfos.addJSON(event); });

        CicadaJSONItem value{};
        value.addValue("url", url);
        value.addArray("events", wantedInfos);
        requestInfoArray.addJSON(value);
    }

    return requestInfoArray.printJSON();
}

void MediaPlayerAnalyticsUtil::filterNetworkInfo(CicadaJSONArray &infos, int64_t timeFrom, int64_t timeTo,
                                                 const std::function<void(CicadaJSONItem &event)> &filter)
{
    int size = infos.getSize();
    for (int i = 0; i < size; i++) {
        CicadaJSONItem &event = infos.getItem(i);
        if (event.hasItem("t")) {
            int64_t time = event.getInt64("t", -1);
            if (time >= timeFrom && time <= timeTo) {
                filter(event);
            }
        }
    }
}

void MediaPlayerAnalyticsUtil::updateBufferInfo(bool force, int64_t videoDuration, int64_t audioDuration)
{
    int64_t time = af_getsteady_ms();
    if (updateBufferInfoLastTimeMs < 0) {
        updateBufferInfoLastTimeMs = time;
    }

    if (force || time - updateBufferInfoLastTimeMs >= 1000) {
        updateBufferInfoLastTimeMs = time;

        CicadaJSONItem bufferInfo{};
        if (videoDuration != INT64_MIN) {
            bufferInfo.addValue("v", (double) videoDuration);
        }

        if (audioDuration != INT64_MIN) {
            bufferInfo.addValue("a", (double) audioDuration);
        }

        std::lock_guard<std::mutex> lockGuard(utilMutex);
        if (mBufferInfo.size() >= MAX_COUNT) {
            mBufferInfo.erase(mBufferInfo.begin());
        }
        mBufferInfo[time] = bufferInfo.printJSON();
    }
}


void MediaPlayerAnalyticsUtil::getPropertyJSONStr(const std::string &name, CicadaJSONArray &array, bool isArray,
                                                  std::deque<StreamInfo *> &streamInfoQueue, demuxer_service *service)
{
    if (nullptr == service) {
        return;
    }

    for (auto &it : streamInfoQueue) {
        std::string str = service->GetProperty(it->streamIndex, name);

        if (str.empty()) {
            continue;
        }

        if (isArray) {
            CicadaJSONArray subArray(str);

            for (int i = 0; i < subArray.getSize(); ++i) {
                CicadaJSONItem &tempItem = subArray.getItem(i);
                addPropertyType(tempItem, it->type);
                array.addJSON(tempItem);
            }
        } else {
            CicadaJSONItem loopItem(str);
            addPropertyType(loopItem, it->type);
            array.addJSON(loopItem);
        }
    }
}

void MediaPlayerAnalyticsUtil::addPropertyType(CicadaJSONItem &item, StreamType type)
{
    switch (type) {
        case ST_TYPE_VIDEO:
            item.addValue("type", "video");
            break;

        case ST_TYPE_AUDIO:
            item.addValue("type", "audio");
            break;

        case ST_TYPE_SUB:
            item.addValue("type", "subtitle");
            break;

        default:
            item.addValue("type", "unknown");
            break;
    }
}

void MediaPlayerAnalyticsUtil::addURLProperty(const std::string &name, CicadaJSONArray &array, IDataSource *dataSource)
{
    if (dataSource) {
        string str = dataSource->GetOption(name);

        if (str.empty()) {
            return;
        }

        CicadaJSONItem item(str);
        item.addValue("type", "url");
        array.addJSON(item);
    }
}
