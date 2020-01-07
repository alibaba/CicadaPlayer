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

AbrBufferRefererData::~AbrBufferRefererData()
{
}


int64_t AbrBufferRefererData::GetMaxBufferDurationInConfig()
{
    playerHandle *handle = (playerHandle *)mHandle;

    if (handle) {
        char maxBufferDur[48] = {0};
        CicadaGetOption(handle, "maxBufferDuration", maxBufferDur);
        return atoll(maxBufferDur);
    }

    return 0;
}

int AbrBufferRefererData::GetRemainSegmentCount()
{
    playerHandle *handle = (playerHandle *)mHandle;

    if (handle) {
        return (int)CicadaGetPropertyLong(handle, PROPERTY_KEY_REMAIN_LIVE_SEG);
    }

    return -1;
}

bool AbrBufferRefererData::GetIsConnected()
{
    playerHandle *handle = (playerHandle *)mHandle;

    if (handle) {
        return (bool)CicadaGetPropertyLong(handle, PROPERTY_KEY_NETWORK_IS_CONNECTED);
    }

    return -1;
}

int64_t AbrBufferRefererData::GetCurrentPacketBufferLength()
{
    playerHandle *handle = (playerHandle *)mHandle;

    if (handle) {
        return CicadaGetPropertyLong(handle, PROPERTY_KEY_VIDEO_BUFFER_LEN);
    }

    return 0;
}
