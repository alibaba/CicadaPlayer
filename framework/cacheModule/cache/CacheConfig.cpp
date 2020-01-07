//
// Created by lifujun on 2019/5/23.
//

#include "CacheConfig.h"

CacheConfig::CacheConfig()
{
}

CacheConfig::~CacheConfig()
{
}

void CacheConfig::reset()
{
    mEnable = false;
    mMaxDurationS = 0;
    mMaxDirSizeMB = 0;
    mCacheDir = "";
    mCacheFileName = "";
    mSourceSize = 0;
}

bool CacheConfig::isSame(const CacheConfig &config)
{
    return config.mEnable == mEnable && config.mMaxDurationS == mMaxDurationS &&
           config.mMaxDirSizeMB == mMaxDirSizeMB && config.mCacheDir == mCacheDir &&
           config.mCacheFileName == mCacheFileName && config.mSourceSize == mSourceSize;
}
