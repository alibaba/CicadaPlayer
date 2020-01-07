//
// Created by lifujun on 2019/5/31.
//

#include "CachePath.h"

#include <utils/file/FileUtils.h>
#include <utils/af_string.h>
#include <utility>
#include <utils/Md5Utils.h>

using namespace Cicada;

#define CACHE_SUFFIX (".alv")

CachePath::CachePath()
{
}

CachePath::~CachePath()
{
}

void CachePath::setCacheConfig(const CacheConfig &config)
{
    mCacheConfig = config;
}

void CachePath::setSourceURL(const string &URL)
{
    mOriginalURL      = URL;
}


string CachePath::getCachedFilePath()
{
    if (!mCacheConfig.mEnable) {
        return "";
    }

    if (AfString::isLocalURL(mOriginalURL)) {
        return "";
    }

    //net file .
    string cachePath = getCachePath(mOriginalURL, mCacheConfig);

    if (FileUtils::isFileExist(cachePath.c_str()) == FILE_TRUE) {
        return cachePath;
    } else {
        return "";
    }
}


string CachePath::getCachePath(const string &URL, CacheConfig &config)
{
    string &cacheDir = config.mCacheDir;

    if (cacheDir.empty()) {
        //no cache dir,no cache files
        return "";
    }

    string cacheName;

    if (!config.mCacheFileName.empty()) {
        cacheName = config.mCacheFileName;
    } else {
        //net file .
        if (URL.empty()) {
            return "";
        }

        cacheName = generateCacheNameByURL(URL);
    }

    string cachePath = cacheDir + PATH_SEPARATION + cacheName + CACHE_SUFFIX;
    return cachePath;
}

string CachePath::generateCacheNameByURL(const string &URL)
{
    if (URL.empty()) {
        return "";
    }

    size_t startIndex = URL.find_first_of(":");
    size_t endIndex = URL.find_first_of("?");

    if (startIndex == string::npos) {
        return "";
    }

    if (endIndex == string::npos) {
        endIndex = URL.length();
    }

    size_t sublen = endIndex - (startIndex + 1);
    string uri = URL.substr(startIndex + 1, sublen);
    return Cicada::Md5Utils::getMd5(uri);
}

string CachePath::getCachePath()
{
    string path = getCachePath(mOriginalURL, mCacheConfig);
    return path;
}

void CachePath::reset()
{
    mOriginalURL = "";
    mCacheConfig.reset();
}

string CachePath::getSourceUrl()
{
    return mOriginalURL;
}

