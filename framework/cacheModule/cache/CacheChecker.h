//
// Created by lifujun on 2019/5/30.
//

#ifndef SOURCE_CACHECHECKER_H
#define SOURCE_CACHECHECKER_H


#include "CacheConfig.h"
#include "CacheRet.h"
#include <vector>


class CacheFileInfo {
public:
    string path;
    long   length;
    long   createTime;
};

class CacheChecker {

public:
    CacheChecker();

    ~CacheChecker();

    void setCacheConfig(const CacheConfig &config);

    CacheRet checkConfig();

    void setMediaInfo(int64_t fileSize, int64_t duration);

    bool checkMediaInfo();

    bool checkSpaceEnough();

    void reset();

private:

    void getAllCachedFiles(const string &cacheDir, std::vector<CacheFileInfo> &cacheFiles);

private:

    int64_t mMediaSize     = 0;
    int64_t mMediaDuration = 0;

    CacheConfig       mCacheConfig;

};


#endif //SOURCE_CACHECHECKER_H
