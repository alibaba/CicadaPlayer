//
// Created by lifujun on 2019/5/30.
//

#ifndef SOURCE_CACHEMODULE_H
#define SOURCE_CACHEMODULE_H


#include "./cache/CacheFileRemuxer.h"
#include "./cache/CacheConfig.h"
#include "./cache/CacheChecker.h"
#include "./cache/CachePath.h"
#include "cache/CacheRet.h"

#include <vector>
#include <mutex>
#include <utils/mediaTypeInternal.h>

using namespace std;


class CacheModule {

public:

    enum CacheStatus{
        idle,fail,success
    };

    CacheModule();

    ~CacheModule();

    /**
     * set cache config
     * @param config
     */
    void setCacheConfig(const CacheConfig &config);

    /**
     * set cache url. be used to generate cache file path
     * @param URL
     */
    void setSourceUrl(const string &URL);

    void setDescription(const string& description);

    string getSourceUrl();
    /**
     * get local cached file path .
     * @return  If file not exists ,or cache not enable, will return empty string
     */
    string getCachedFilePath();

//    string getActualPlayURL();
//
//    bool isActualPlayCache();

    void setMediaInfo(int64_t fileSize, int64_t duration);

    bool isMediaInfoSet();

    void addFrame(const unique_ptr<IAFPacket>& frame, StreamType type);

    void setErrorCallback(function<void(int, string)> callback);

    void setResultCallback(function<void(bool)> callback);

    CacheRet checkCanBeCached(const string &acturalPlayURL);

    CacheRet start();
    /**
     * stream end , means a completion.
     */
    void streamEnd();

    /**
     * if streamEnd is false , stop will remove cached file.
     */
    void stop();

    CacheStatus getCacheStatus();

    void reset();

    void setStreamMeta(const vector<Stream_meta*>& streamMetas);

    void clearStreamMetas();
private:


    CacheRet checkCanCache();

    enum Status {
        Prepared, Stopped
    };

private:

    bool mMediaInfoSet = false;
    CacheStatus mCacheRet = CacheStatus::idle;

    mutex  mStatusMutex;
    Status mStatus     = Status::Stopped;

    mutex            mReumxerMutex;
    CacheFileRemuxer *mCacheFileRemuxer = nullptr;

    CacheChecker      mCacheChecker;
    CachePath         mCachePath;
    string mDescription;

    function<void(int, string)> mErrorCallback = nullptr;
    function<void(bool)> mResultCallback = nullptr;

    vector<Stream_meta*> mStreamMetas{};

};


#endif //SOURCE_CACHEMODULE_H
