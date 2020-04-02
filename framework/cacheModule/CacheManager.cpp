//
// Created by lifujun on 2019/9/24.
//
#define LOG_TAG "CacheManager"

#include "CacheManager.h"
#include <utility>
#include <utils/mediaTypeInternal.h>
#include <vector>
#include <utils/mediaFrame.h>

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

void CacheManager::sendMediaFrame(const unique_ptr<IAFPacket> &frame, StreamType type)
{
    if (!mNeedProcessFrame) {
        return;
    }

    bool isMediaInfoSet = mCacheModule.isMediaInfoSet();

    if (!isMediaInfoSet) {
        int64_t streamSize = mDataSource->getStreamSize();
        int64_t duration   = mDataSource->getDuration();
        mCacheModule.setMediaInfo(streamSize, duration);
        vector<Stream_meta *> streamMetas{};
        int metaRet = -1;
        {
            auto *videoMeta = static_cast<Stream_meta *>(malloc(sizeof(Stream_meta)));
            memset(videoMeta, 0, sizeof(Stream_meta));
            metaRet = mDataSource->getStreamMeta(videoMeta, StreamType::ST_TYPE_VIDEO);

            if (metaRet == 0) {
                videoMeta->type = Stream_type::STREAM_TYPE_VIDEO;
                streamMetas.push_back(videoMeta);
            }else {
                releaseMeta(videoMeta);
                free(videoMeta);
            }
        }
        {
            auto *audioMeta = static_cast<Stream_meta *>(malloc(sizeof(Stream_meta)));
            memset(audioMeta, 0, sizeof(Stream_meta));
            metaRet = mDataSource->getStreamMeta(audioMeta, StreamType::ST_TYPE_AUDIO);

            if (metaRet == 0) {
                audioMeta->type = Stream_type::STREAM_TYPE_AUDIO;
                streamMetas.push_back(audioMeta);
            } else {
                releaseMeta(audioMeta);
                free(audioMeta);
            }
        }
        mCacheModule.setStreamMeta(streamMetas);
        mCacheModule.setErrorCallback([this](int code, string msg) -> void{
            AF_LOGE("cacheModule error : code = %d , msg = %s ", code, msg.c_str());
            mNeedProcessFrame = false;

            if (mCacheFailCallback != nullptr) {
                mCacheFailCallback(code, msg);
            }
        });
        mCacheModule.setResultCallback([this](bool success) -> void {
            if (success) {
                if (mCacheSuccessCallback != nullptr) {
                    mCacheSuccessCallback();
                }
            } else {
                if (mCacheFailCallback != nullptr) {
                    mCacheFailCallback(-1, mStopReason);
                }
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
    mNeedProcessFrame = false;

    std::unique_lock<mutex> lock(mStopMutex);
    mStopReason = reason;
    mCacheModule.stop();
}

void CacheManager::complete()
{
    mCacheModule.streamEnd();
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

