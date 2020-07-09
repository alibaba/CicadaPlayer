//
// Created by lifujun on 2019/5/30.
//

#define LOG_TAG "CacheChecker"

#include <utility>
#include <dirent.h>
#include <algorithm>
#include "CacheChecker.h"
#include <utils/file/FileUtils.h>
#include <utils/frame_work_log.h>
#include <vector>
#include <cstring>

#define CACHE_SUFFIX (".alv")
#define CACHE_SUFFIX_OLD (".mp4")
#define CACHE_SUFFIX_TMP (".tmp")

using namespace Cicada;

CacheChecker::CacheChecker()
{
}

CacheChecker::~CacheChecker()
{
}

void CacheChecker::setCacheConfig(const CacheConfig &config)
{
    mCacheConfig = config;
}

CacheRet CacheChecker::checkConfig()
{
    if (!mCacheConfig.mEnable) {
        AF_LOGE("cache config not enable");
        return CACHE_ERROR_NOT_ENABLE;
    }

    string &cacheDir = mCacheConfig.mCacheDir;

    if (cacheDir.empty()) {
        //not set cache dir
        AF_LOGE("cache config not set cache dir");
        return CACHE_ERROR_CACHE_DIR_EMPTY;
    }

    if (FileUtils::isDirExist(cacheDir.c_str()) != FILE_TRUE) {
        //create cache dir if not exits.
        int ret = FileUtils::mkdirs(cacheDir.c_str());

        if (ret == FILE_TRUE) {
            return CACHE_SUCCESS;
        } else {
            return CACHE_ERROR_CACHE_DIR_ERROR;
        }
    } else {
        return CACHE_SUCCESS;
    }
}

void CacheChecker::setMediaInfo(int64_t fileSize, int64_t duration)
{
    mMediaSize = fileSize;
    mMediaDuration = duration;
}

bool CacheChecker::checkMediaInfo()
{
    int64_t duration = mMediaDuration;
    int64_t fileSize = mMediaSize;

    if (duration > mCacheConfig.mMaxDurationS * 1000) {
        return false;
    }

    if (fileSize <= 0) {
        fileSize = mCacheConfig.mSourceSize;
    }

    if (fileSize <= 0) {
        return false;
    }

    //1024*1024 = 1048576
    if (fileSize / 1048576.0f > mCacheConfig.mMaxDirSizeMB) {
        //file size over large.
        return false;
    }

    return true;
}

bool CacheChecker::checkSpaceEnough()
{
    //list old cache files and remove old files to get enough space.
    vector<CacheFileInfo> cacheFiles;
    getAllCachedFiles(mCacheConfig.mCacheDir, cacheFiles);

    if (cacheFiles.empty()) {
        return true;
    }

    int64_t totalCacheLen = 0;

    for (const CacheFileInfo &cacheFileInfo : cacheFiles) {
        totalCacheLen += cacheFileInfo.length;
    }

    int64_t MAX_DIR_SIZE = mCacheConfig.mMaxDirSizeMB * 1024 * 1024;

    if (totalCacheLen + mMediaSize < MAX_DIR_SIZE) {
        //space is enough
        return true;
    }

    int64_t leftSpaceIfDeleteFile = totalCacheLen;

    for (const CacheFileInfo &cacheFileInfo : cacheFiles) {
        leftSpaceIfDeleteFile -= cacheFileInfo.length;

        if (leftSpaceIfDeleteFile + mMediaSize < MAX_DIR_SIZE) {
            //remove this , space is enough
            int ret = FileUtils::rmrf(cacheFileInfo.path.c_str());
            return (ret == FILE_TRUE);
        } else {
            //remove this is NOT enough yet，remove and continue
            int ret = FileUtils::rmrf(cacheFileInfo.path.c_str());
        }
    }

    return false;
}


bool compare(const CacheFileInfo &a, const CacheFileInfo &b)
{
    return a.createTime < b.createTime;
}

void CacheChecker::getAllCachedFiles(const string &cacheDir, vector<CacheFileInfo> &cacheFileInfos)
{
    DIR *dir;
    struct dirent *entry;
    char path[UTILS_PATH_MAX + 1] = {0};
    dir = opendir(cacheDir.c_str());
    if (dir == nullptr) {
        return;
    }

    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp((const char *) (entry->d_name), ".") &&
                strcmp((const char *) (entry->d_name), (const char *) "..")) {
            snprintf(path, (size_t) UTILS_PATH_MAX, "%s%c%s", cacheDir.c_str(), PATH_SEPARATION,
                     entry->d_name);
        }

        string tmpFilePath = path;
        unsigned long pathLen = tmpFilePath.length();
        int SUFFIX_LEN = static_cast<int>(strlen(CACHE_SUFFIX));
        int SUFFIX_OLD_LEN = static_cast<int>(strlen(CACHE_SUFFIX_OLD));
        int SUFFIX_TMP_LEN = static_cast<int>(strlen(CACHE_SUFFIX_TMP));
        unsigned long cacheSuffixPos = tmpFilePath.rfind(CACHE_SUFFIX);
        unsigned long oldCacheSuffixPos = tmpFilePath.rfind(CACHE_SUFFIX_OLD);
        unsigned long tmpCacheSuffixPos = tmpFilePath.rfind(CACHE_SUFFIX_TMP);

        if (cacheSuffixPos == pathLen - SUFFIX_LEN ||
                oldCacheSuffixPos == pathLen - SUFFIX_OLD_LEN ||
                tmpCacheSuffixPos == pathLen - SUFFIX_TMP_LEN) {
            //ok, is cache file ..
            CacheFileInfo cacheFileInfo;
            cacheFileInfo.path = tmpFilePath;
            cacheFileInfo.length = FileUtils::getFileLength(path);
            cacheFileInfo.createTime = FileUtils::getFileCreateTime(path);
            AF_LOGD("find a cache file .. path = %s , createTime = %lld",
                    cacheFileInfo.path.c_str(), cacheFileInfo.createTime);
            cacheFileInfos.push_back(cacheFileInfo);
        }
    }

    sort(cacheFileInfos.begin(), cacheFileInfos.end(), compare);

    closedir(dir);
}

void CacheChecker::reset()
{
    mMediaSize = 0;
    mMediaDuration = 0;
    mCacheConfig.reset();
}

