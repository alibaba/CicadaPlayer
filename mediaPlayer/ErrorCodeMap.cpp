//
// Created by lifujun on 2019/3/14.
//

#define LOG_TAG "ErrorCodeMap"
#include "ErrorCodeMap.h"

#include <mutex>
#include "media_player_error_def.h"
#include "utils/frame_work_log.h"

using  namespace Cicada;
ErrorCodeMap *ErrorCodeMap::sInstance = nullptr;

ErrorCodeMap *ErrorCodeMap::getInstance()
{
    static std::once_flag oc;
    std::call_once(oc, [&] {
        sInstance = new ErrorCodeMap();
    });
    return sInstance;
}

ErrorCodeMap::ErrorCodeMap()
{
    init();
}

ErrorCodeMap::~ErrorCodeMap()
{
}


void ErrorCodeMap::init()
{
    //==========================
    // 2、apsaraplayer;
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_NATIVE_START, 0x20020000));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_READ_PACKET, 0x20020001));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_BUFFER_STUFFED, 0x20020002));
    // 3、demuxer、datasource
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_DEMUXER_START, 0x20030000));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_DEMUXER_OPENURL, 0x20030001));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_DEMUXER_NO_VALID_STREAM, 0x20030002));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_DEMUXER_OPENSTREAM, 0x20030003));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_LOADING_TIMEOUT, 0x20030004));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_DATASOURCE_EMPTYURL, 0x20030005));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_DECODE_BASE, 0x20040000));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_DECODE_VIDEO, 0x20040001));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_DECODE_AUDIO, 0x20040002));
    // 4. network
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_NETWORK_UNKNOWN, 0x20050000));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_NETWORK_UNSUPPORTED, 0x20050001));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_NETWORK_RESOLVE, 0x20050002));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_NETWORK_CONNECT_TIMEOUT, 0x20050003));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_NETWORK_COULD_NOT_CONNECT, 0x20050004));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_NETWORK_HTTP_403, 0x20050005));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_NETWORK_HTTP_404, 0x20050006));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_NETWORK_HTTP_4XX, 0x20050007));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_NETWORK_HTTP_5XX, 0x20050008));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_NETWORK_HTTP_RANGE, 0x20050009));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_NETWORK_HTTP_400, 0x2005000A));
    //5. codec
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_CODEC_UNKNOWN, 0x20060000));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_CODEC_VIDEO_NOT_SUPPORT, 0x20060001));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_CODEC_AUDIO_NOT_SUPPORT, 0x20060002));
    //6. internal
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_INERNAL_UNKNOWN, 0x20070000));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_INERNAL_EXIT, 0x20070001));
    //7.general
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_GENERAL_UNKNOWN, 0x20080000));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_GENERAL_EPERM, 0x20080001));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_GENERAL_ENOENT, 0x20080002));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_GENERAL_EIO, 0x20080005));
    //8.render
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_RENDER_AUDIO_OPEN_DEVICE_FAILED, 0x20090001));
    //
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_ERROR_UNKNOWN, 0x30000000 - 1));
}

int ErrorCodeMap::getValue(int orignalValue)
{
    if (codeMap.count(orignalValue) > 0) {
        return codeMap.at(orignalValue);
    } else {
        AF_LOGE("not mapped errorCode : %d ", orignalValue);
        return orignalValue;
    }
}

bool ErrorCodeMap::containsCode(int orignalValue)
{
    return (codeMap.count(orignalValue) > 0);
}
