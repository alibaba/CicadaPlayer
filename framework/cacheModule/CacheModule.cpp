//
// Created by lifujun on 2019/5/30.
//

#define  LOG_TAG "CacheModule"

#include <utils/file/FileUtils.h>
#include <utils/af_string.h>
#include "CacheModule.h"
#include <utility>
#include <dirent.h>
#include <utils/mediaFrame.h>
#include "utils/frame_work_log.h"

#define TMP_SUFFIX (".tmp")

using namespace Cicada;

CacheModule::CacheModule()
{
}

CacheModule::~CacheModule()
{
    stop();
    clearStreamMetas();
    delete mCacheFileRemuxer;
}

void CacheModule::setCacheConfig(const CacheConfig &config)
{
    AF_LOGD("---> setCacheConfig()");
    mCacheChecker.setCacheConfig(config);
    mCachePath.setCacheConfig(config);
}

void CacheModule::setSourceUrl(const string &URL)
{
    AF_LOGD("---> setOriginalPlayInfo() url = %s", URL.c_str());
    mCachePath.setSourceURL(URL);
}

void CacheModule::setDescription(const string &description)
{
    AF_LOGD("---> setDescription() description = %s", description.c_str());
    mDescription = description;
}

string CacheModule::getSourceUrl()
{
    return mCachePath.getSourceUrl();;
}

string CacheModule::getCachedFilePath()
{
    return mCachePath.getCachedFilePath();
}

CacheRet CacheModule::checkCanBeCached(const string &acturalPlayURL)
{
    AF_LOGD("---> prepare()");
    {
        unique_lock<mutex> lock(mStatusMutex);

        if (mStatus != Stopped) {
            return CACHE_ERROR_STATUS;
        }

        if (AfString::isLocalURL(acturalPlayURL)) {
            mStatus = Status::Stopped;
            AF_LOGE("url is local source");
            return CACHE_ERROR_LOCAL_SOURCE;
        }

        const CacheRet &cacheRet = mCacheChecker.checkConfig();
        mStatus = (cacheRet.mCode == CACHE_SUCCESS.mCode ? Status::Prepared : Status::Stopped);
        return cacheRet;
    }
}

void CacheModule::setMediaInfo(int64_t fileSize, int64_t duration)
{
    AF_LOGD("---> setMediaInfo()");
    mMediaInfoSet = true;
    mCacheChecker.setMediaInfo(fileSize, duration);
}

bool CacheModule::isMediaInfoSet()
{
    return mMediaInfoSet;
}

void CacheModule::setErrorCallback(function<void(int, string)> callback)
{
    mErrorCallback = std::move(callback);
}

void CacheModule::setResultCallback(function<void(bool)> callback)
{
    mResultCallback = std::move(callback);
}

CacheRet CacheModule::start()
{
    {
        unique_lock<mutex> lock(mStatusMutex);

        if (mStatus == Status::Stopped) {
            AF_LOGE("---> start()  , mStatus == Status::Stopped return ");
            return CACHE_ERROR_STATUS;
        }
    }
    AF_LOGD("---> start()");
    const CacheRet &canCheck = checkCanCache();

    if (canCheck.mCode != CACHE_SUCCESS.mCode) {
        AF_LOGE("---> start()  , checkCanCache fail.. return ");
        return canCheck;
    }

    {
        std::unique_lock<mutex> remuxerLock(mReumxerMutex);

        if (mCacheFileRemuxer != nullptr) {
            mCacheFileRemuxer->interrupt();
            mCacheFileRemuxer->stop();
            delete mCacheFileRemuxer;
            mCacheFileRemuxer = nullptr;
        }

        string cacheTmpPath = mCachePath.getCachePath() + TMP_SUFFIX;
        mCacheFileRemuxer = new CacheFileRemuxer(cacheTmpPath, mDescription);
        mCacheFileRemuxer->setStreamMeta(&mStreamMetas);
        mCacheFileRemuxer->setErrorCallback([this](int code, string msg) -> void {
            if (mErrorCallback != nullptr) {
                mErrorCallback(code, msg);
            }
        });
        mCacheFileRemuxer->setResultCallback([this](bool success) -> void {
            const string &cachePath = mCachePath.getCachePath();
            string cacheTmpFilePath = cachePath + TMP_SUFFIX;
            if (success) {
                int ret = FileUtils::Rename(cacheTmpFilePath.c_str(), cachePath.c_str());

                if (ret == 0) {
                    mCacheRet = CacheStatus::success;
                } else {
                    FileUtils::rmrf(cacheTmpFilePath.c_str());
                    mCacheRet = CacheStatus::fail;
                }
            } else {
                // not completion ,  need delete cached file
                FileUtils::rmrf(cacheTmpFilePath.c_str());
                mCacheRet = CacheStatus::fail;
            }

            if (mResultCallback != nullptr) {
                mResultCallback(success);
            }
        });
        bool prepareSucced = mCacheFileRemuxer->prepare();

        if (!prepareSucced) {
            AF_LOGE("---> start()  , cacheFileRemuxer->prepare() fail");
            delete mCacheFileRemuxer;
            mCacheFileRemuxer = nullptr;
            return CACHE_ERROR_FILE_REMUXER_OPEN_ERROR;
        }

        AF_LOGD("---> start()  , cacheFileRemuxer->start()");
        mCacheFileRemuxer->start();
    }

    return CACHE_SUCCESS;
}

CacheRet CacheModule::checkCanCache()
{
    AF_LOGD("---> checkCanCache()");
    bool matchConfig = mCacheChecker.checkMediaInfo();

    if (!matchConfig) {
        // not match with config
        AF_LOGE("---> checkCanCache() not match with config");
        return CACHE_ERROR_MEDIA_INFO_NOT_MATCH;
    }

    bool spaceEnough = mCacheChecker.checkSpaceEnough();

    if (!spaceEnough) {
        AF_LOGE("---> checkCanCache() space not Enough");
        return CACHE_ERROR_MEDIA_INFO_NOT_MATCH;
    }

    return CACHE_SUCCESS;
}


void CacheModule::addFrame(const unique_ptr<IAFPacket> &frame, StreamType type)
{
    std::unique_lock<mutex> lock(mReumxerMutex);

    if (mCacheFileRemuxer != nullptr) {
        mCacheFileRemuxer->addFrame(frame, type);
    }
}

void CacheModule::streamEnd()
{
    AF_LOGD("---> streamEnd()");
    addFrame(nullptr, StreamType::ST_TYPE_UNKNOWN);
}

void CacheModule::stop()
{
    {
        unique_lock<mutex> lock(mStatusMutex);
        mStatus = Status::Stopped;
    }
    AF_LOGD("---> stop()");
    {
        std::unique_lock<mutex> remuxerLock(mReumxerMutex);
        mCacheRet = CacheStatus::idle;

        if (mCacheFileRemuxer != nullptr) {
            mCacheFileRemuxer->interrupt();
            mCacheFileRemuxer->stop();
        }
    }
}

void CacheModule::reset()
{
    AF_LOGD("---> reset()");
    unique_lock<mutex> lock(mStatusMutex);
    mMediaInfoSet = false;
    mCacheRet = CacheStatus::idle;;
    mCacheChecker.reset();
    mCachePath.reset();
}

CacheModule::CacheStatus CacheModule::getCacheStatus()
{
    AF_LOGD("<---- getCacheStatus() %d", mCacheRet);
    return mCacheRet;
}

void CacheModule::setStreamMeta(const vector<Stream_meta *> &streamMetas)
{
    clearStreamMetas();

    if (streamMetas.empty()) {
        return;
    }

    for (auto &item : streamMetas) {
        mStreamMetas.push_back(item);
    }
}

void CacheModule::clearStreamMetas()
{
    if (!mStreamMetas.empty()) {
        for (auto &item : mStreamMetas) {
            releaseMeta(item);
            free(item);
        }

        mStreamMetas.clear();
    }
}

