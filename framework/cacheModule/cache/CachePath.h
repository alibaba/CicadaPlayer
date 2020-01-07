//
// Created by lifujun on 2019/5/31.
//

#ifndef SOURCE_CACHEPATH_H
#define SOURCE_CACHEPATH_H


#include "CacheConfig.h"
#include <cstring>

using namespace std;

class CachePath {
public:
    CachePath();

    ~CachePath();

    void setCacheConfig(const CacheConfig &config);

    void setSourceURL(const string &URL);

    string getCachePath();

    void reset();

    string getCachedFilePath();

    string getSourceUrl();

public:

    static string getCachePath(const string &URL, CacheConfig& config);


private:
    static string generateCacheNameByURL(const string &URL);

private:

    string            mOriginalURL;
    CacheConfig       mCacheConfig;
};


#endif //SOURCE_CACHEPATH_H
