//
// Created by lifujun on 2019/9/24.
//
#define LOG_TAG "CacheManager"

#include "CacheManager.h"
#include <utility>

CacheManager::CacheManager()
{
}

CacheManager::~CacheManager()
{
    if (mDataSource != nullptr) {
        delete mDataSource;
        mDataSource = nullptr;
    }
}

void CacheManager::setCacheConfig(const CacheConfig &config)
{
    mCacheConfig = config;
}

void CacheManager::setSourceUrl(const string &url)
{
    mSourceUrl = url;
}

void CacheManager::setDescription(const string &description)
{
    mDescription = description;
}


void CacheManager::setDataSource(ICacheDataSource *dataSource)
{
    mDataSource = dataSource;
}

string CacheManager::init()
{
    if (!mCacheConfig.mEnable) {
        return mSourceUrl;
    }

    mCacheModule.setCacheConfig(mCacheConfig);
    mCacheModule.setSourceUrl(mSourceUrl);
    mCacheModule.setDescription(mDescription);
    mNeedProcessFrame = false;
    string cacheFilePath = mCacheModule.getCachedFilePath();

    if (!cacheFilePath.empty()) {
        return cacheFilePath;
    }

    //not cached yet, check if can cache this url
    const CacheRet &canBeCached = mCacheModule.checkCanBeCached(mSourceUrl);
    AF_LOGD("canBeCached = %d , SourceUrl = %s", canBeCached.mCode, mSourceUrl.c_str());

    if (canBeCached.mCode == CACHE_SUCCESS.mCode) {
        mNeedProcessFrame = true;
    }

    return mSourceUrl;
}

void CacheManager::sendMediaFrame(const unique_ptr<IAFPacket>& frame, StreamType type)
{
    if (!mNeedProcessFrame) {
        return;
    }

    bool isMediaInfoSet = mCacheModule.isMediaInfoSet();

    if (!isMediaInfoSet) {
        int64_t streamSize = mDataSource->getStreamSize();
        int64_t duration   = mDataSource->getDuration();
        mCacheModule.setMediaInfo(streamSize, duration);
        mCacheModule.setMetaCallback([this](StreamType type, Stream_meta * meta) -> bool {
            int metaRet = mDataSource->getStreamMeta(meta, type);
            return (metaRet == 0);
        });
        mCacheModule.setErrorCallback([this](int code, string msg) -> void{
            AF_LOGE("cacheModule error : code = %d , msg = %s ", code, msg.c_str());

            if (mCacheFailCallback != nullptr)
            {
                mCacheFailCallback(code, msg);
            }
        });
        const CacheRet &startRet = mCacheModule.start();

        if (startRet.mCode != CACHE_SUCCESS.mCode) {
            mNeedProcessFrame = false;
            if (mCacheFailCallback != nullptr) {
                mCacheFailCallback(startRet.mCode, startRet.mMsg);
            }

            return;
        }
    }

    mCacheModule.addFrame(frame, type);
}



void CacheManager::stop(const string &reason)
{
    mCacheModule.stop();
    CacheModule::CacheStatus status = mCacheModule.getCacheStatus();
    mNeedProcessFrame = false;
    mCacheModule.reset();

    if (status == CacheModule::CacheStatus::fail) {
        if (mCacheFailCallback != nullptr) {
            mCacheFailCallback(-1, reason);
        }
    }
}

void CacheManager::complete()
{
    if (mCacheConfig.mEnable) {
        mCacheModule.streamEnd();
        CacheModule::CacheStatus cacheStatus = mCacheModule.getCacheStatus();

        if (cacheStatus == CacheModule::success) {
            if (mCacheSuccessCallback != nullptr) {
                mCacheSuccessCallback();
            }
        }

        AF_LOGD("eventCallback ==== cacheComplete cacheSuccess is %d", cacheStatus);
    }
}

CacheModule::CacheStatus CacheManager::getCacheStatus()
{
    return mCacheModule.getCacheStatus();
}

string CacheManager::getSourceUrl()
{
    return mCacheModule.getSourceUrl();
}

string CacheManager::getCachePath(const string &url, CacheConfig &config)
{
    return CachePath::getCachePath(url, config);
}

void CacheManager::setCacheSuccessCallback(function<void()> resultCallback)
{
    mCacheSuccessCallback = std::move(resultCallback);
}

void CacheManager::setCacheFailCallback(function<void(int, string)> resultCallback)
{
    mCacheFailCallback = std::move(resultCallback);
}

