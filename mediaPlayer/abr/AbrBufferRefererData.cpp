//
//  AbrBufferRefererData.cpp
//  apsara_player
//
//  Created by shiping.csp on 2018/11/1.
//

#include "AbrBufferRefererData.h"
#include "media_player_api.h"
#include "native_cicada_player_def.h"
#include <cstdlib>

AbrBufferRefererData::AbrBufferRefererData(void *handle)
{
    mHandle = handle;
}

AbrBufferRefererData::~AbrBufferRefererData() = default;


int64_t AbrBufferRefererData::GetMaxBufferDurationInConfig()
{
    auto *handle = (playerHandle *) mHandle;

    if (handle) {
        char maxBufferDur[48] = {0};
        CicadaGetOption(handle, "maxBufferDuration", maxBufferDur);
        return atoll(maxBufferDur);
    }

    return 0;
}

int AbrBufferRefererData::GetRemainSegmentCount()
{
    auto *handle = (playerHandle *) mHandle;

    if (handle) {
        return (int) CicadaGetPropertyLong(handle, PROPERTY_KEY_REMAIN_LIVE_SEG);
    }

    return -1;
}

bool AbrBufferRefererData::GetIsConnected()
{
    auto *handle = (playerHandle *) mHandle;

    if (handle) {
        return (bool) CicadaGetPropertyLong(handle, PROPERTY_KEY_NETWORK_IS_CONNECTED);
    }

    return -1;
}

int64_t AbrBufferRefererData::GetCurrentPacketBufferLength()
{
    auto *handle = (playerHandle *) mHandle;

    if (handle) {
        return CicadaGetPropertyLong(handle, PROPERTY_KEY_VIDEO_BUFFER_LEN);
    }

    return 0;
}
bool AbrBufferRefererData::GetReBuffering()
{
    auto *handle = (playerHandle *) mHandle;

    if (handle) {
        return CicadaGetPropertyLong(handle, PROPERTY_KEY_RE_BUFFERING);
    }
    return false;
}
