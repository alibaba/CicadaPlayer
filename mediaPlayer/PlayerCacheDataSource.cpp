//
// Created by lifujun on 2019/9/24.
//

#include "PlayerCacheDataSource.h"
#include <media_player_api.h>
#include <cstdlib>

PlayerCacheDataSource::PlayerCacheDataSource(void *playerHandle)
{
    mPlayerHandle = playerHandle;
}

PlayerCacheDataSource::~PlayerCacheDataSource() = default;

void PlayerCacheDataSource::setFrameCallback(playerMediaFrameCb callback, void *userArgs)
{
    auto *player = static_cast<playerHandle *>(mPlayerHandle);
    CicadaSetMediaFrameCb(player, callback, userArgs);
}

int64_t PlayerCacheDataSource::getStreamSize()
{
    auto *player = static_cast<playerHandle *>(mPlayerHandle);
    char streamSizeStr[256] = {0};
    CicadaGetOption(player, "mediaStreamSize", streamSizeStr);
    return atoll(streamSizeStr);
}

int64_t PlayerCacheDataSource::getDuration()
{
    auto *player = static_cast<playerHandle *>(mPlayerHandle);
    return CicadaGetDuration(player);
}

int PlayerCacheDataSource::getStreamMeta(Stream_meta *ptr, StreamType type)
{
    auto *player = static_cast<playerHandle *>(mPlayerHandle);
    return CicadaGetCurrentStreamMeta(player, ptr, type);
}
