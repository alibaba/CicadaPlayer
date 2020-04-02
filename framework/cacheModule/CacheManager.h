//
// Created by lifujun on 2019/9/24.
//

#ifndef SOURCE_CACHEMANAGER_H
#define SOURCE_CACHEMANAGER_H


#include <cacheModule/cache/CacheConfig.h>
#include <cacheModule/CacheModule.h>

class ICacheDataSource {

public :
    virtual ~ICacheDataSource() = default;

    virtual int64_t getStreamSize() = 0;

    virtual int64_t getDuration() = 0;

    virtual int getStreamMeta(Stream_meta *ptr, StreamType type) = 0;

};

class CacheManager {

public:
    CacheManager();

    ~CacheManager();

    void setCacheConfig(const CacheConfig &config);

    void setDescription(const string &description);

    void setSourceUrl(const string &url);

    string getSourceUrl();

    void setDataSource(ICacheDataSource *dataSource);

    string init();

    void stop(const string &reason);

    void complete();

    CacheModule::CacheStatus getCacheStatus();

    static string getCachePath(const string &url, CacheConfig &config);

    void setCacheFailCallback(function<void(int, string)> resultCallback);

    void setCacheSuccessCallback(function<void()> resultCallback);

    void sendMediaFrame(const std::unique_ptr<IAFPacket> &frame, StreamType type);

private:
    std::mutex mStopMutex{};

    std::atomic_bool mNeedProcessFrame{false};
    string mStopReason{};
    CacheModule mCacheModule;
    CacheConfig mCacheConfig;
    string mSourceUrl;
    string mDescription;
    ICacheDataSource *mDataSource = nullptr;
    function<void(int, string)> mCacheFailCallback = nullptr;
    function<void()> mCacheSuccessCallback = nullptr;

};


#endif //SOURCE_CACHEMANAGER_H
