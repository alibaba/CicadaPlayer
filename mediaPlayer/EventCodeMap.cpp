//
// Created by lifujun on 2019/4/3.
//

#include <mutex>
#include <utils/frame_work_log.h>
#include "EventCodeMap.h"
#include "media_player_error_def.h"

using  namespace Cicada;

EventCodeMap *EventCodeMap::sEventCodeInstance = nullptr;

EventCodeMap *EventCodeMap::getInstance()
{
    static std::once_flag oc;
    std::call_once(oc, [&] {
        sEventCodeInstance = new EventCodeMap();
    });
    return sEventCodeInstance;
}

int EventCodeMap::getValue(int orignalValue)
{
    if (codeMap.count(orignalValue) > 0) {
        return codeMap.at(orignalValue);
    } else {
        AF_LOGE("not mapped eventCode : %d ", orignalValue);
        return orignalValue;
    }
}

EventCodeMap::EventCodeMap()
{
    init();
}

EventCodeMap::~EventCodeMap()
{
}

void EventCodeMap::init()
{
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_EVENT_SW_VIDEO_DECODER, 100));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_EVENT_AUDIO_CODEC_NOT_SUPPORT, 101));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_EVENT_AUDIO_DECODER_DEVICE_ERROR, 102));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_EVENT_VIDEO_CODEC_NOT_SUPPORT, 103));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_EVENT_VIDEO_DECODER_DEVICE_ERROR, 104));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_EVENT_VIDEO_RENDER_INIT_ERROR, 105));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_EVENT_DEMUXER_TRACE_ID, 106));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_EVENT_DEMUXER_EOF, 107));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_EVENT_NETWORK_RETRY, 108));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_EVENT_CACHE_SUCCESS, 109));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_EVENT_CACHE_ERROR, 110));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_EVENT_SYSTEM_LOW_MEMORY, 111));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_EVENT_DEMUXER_STARTUP_INFO, 112));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_EVENT_NETWORK_RETRY_SUCCESS, 113));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_EVENT_SUBTITLE_SELECT_ERROR, 114));
    codeMap.insert(pair<int, int>(MEDIA_PLAYER_EVENT_DECODER_RECOVER_SIZE, 115));
}
