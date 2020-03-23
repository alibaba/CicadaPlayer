//
// Created by lifujun on 2019/5/23.
//

#ifndef SOURCE_CACHECONFIG_H
#define SOURCE_CACHECONFIG_H


#include <string>

using namespace std;

class CacheConfig {
public:
    CacheConfig() = default;

    ~CacheConfig() = default;

    void reset()
    {
        mEnable = false;
        mMaxDurationS = 0;
        mMaxDirSizeMB = 0;
        mCacheDir = "";
        mCacheFileName = "";
        mSourceSize = 0;
    };

    bool isSame(const CacheConfig &config) const
    {
        return config.mEnable == mEnable && config.mMaxDurationS == mMaxDurationS &&
               config.mMaxDirSizeMB == mMaxDirSizeMB && config.mCacheDir == mCacheDir &&
               config.mCacheFileName == mCacheFileName && config.mSourceSize == mSourceSize;
    }

//    std::string toString() const;
public:
    bool mEnable = false;
    int64_t mMaxDurationS = 0;
    int64_t mMaxDirSizeMB = 0;
    string mCacheDir;

    string mCacheFileName;
    int64_t mSourceSize = 0;

};


#endif //SOURCE_CACHECONFIG_H
