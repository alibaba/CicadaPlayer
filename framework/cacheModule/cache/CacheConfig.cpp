//
// Created by lifujun on 2019/5/23.
//

#include "CacheConfig.h"
#include <utils/CicadaJSON.h>

CacheConfig::CacheConfig()
    = default;

CacheConfig::~CacheConfig()
    = default;

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

std::string CacheConfig::toString() const
{
    CicadaJSONItem item{};
    item.addValue("mEnable", mEnable);
    item.addValue("mMaxDurationS", (long)mMaxDurationS);
    item.addValue("mMaxDirSizeMB", (long)mMaxDirSizeMB);
    item.addValue("mCacheDir", mCacheDir);
    item.addValue("mCacheFileName", mCacheFileName);
    item.addValue("mSourceSize", (long)mSourceSize);
    return item.printJSON();
}
