//
// Created by lifujun on 2019/6/10.
//

#ifndef SOURCE_CACHERET_H
#define SOURCE_CACHERET_H

class CacheRet {
public:
    CacheRet(int code, const string &msg) {
        mCode = code;
        mMsg = msg;
    }

    ~CacheRet() = default;

public:
    int mCode;
    string mMsg;
};

static CacheRet CACHE_SUCCESS(0, "");
static CacheRet CACHE_ERROR_STATUS(1, "cache status wrong");
static CacheRet CACHE_ERROR_MUXER_OPEN(2, "muxer open fail");
static CacheRet CACHE_ERROR_MUX_STREAM(3, "mux stream error");
static CacheRet CACHE_ERROR_MUXER_CLOSE(4, "muxer close fail");
static CacheRet CACHE_ERROR_NO_SPACE(5, "don't have enough space");
static CacheRet CACHE_ERROR_LOCAL_SOURCE(6, "url is local source");
static CacheRet CACHE_ERROR_NOT_ENABLE(7, "cache not enable");
static CacheRet CACHE_ERROR_CACHE_DIR_EMPTY(8, "cache dir is empty");
static CacheRet CACHE_ERROR_CACHE_DIR_ERROR(9, "cache dir is error");
static CacheRet CACHE_ERROR_ENCRYPT_CHECK_FAIL(10, "encrypt check fail");
static CacheRet CACHE_ERROR_MEDIA_INFO_NOT_MATCH(11, "media info not match config");
static CacheRet CACHE_ERROR_FILE_REMUXER_OPEN_ERROR(12, "cache file open error");

#endif //SOURCE_CACHERET_H
