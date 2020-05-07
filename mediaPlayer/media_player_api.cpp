#include <complex>
#include <utils/af_string.h>
#include <utils/frame_work_log.h>
#include "media_player_api.h"
#include "ICicadaPlayer.h"
#include "SuperMediaPlayer.h"

using namespace Cicada;

typedef struct playerHandle_t {
    ICicadaPlayer *pPlayer;
} playerHandle_t;

#define GET_PLAYER ICicadaPlayer *player = pHandle->pPlayer

playerHandle *CicadaCreatePlayer()
{
    playerHandle *pHandle = new playerHandle();
    pHandle->pPlayer = new SuperMediaPlayer();
    return pHandle;
}

void CicadaReleasePlayer(playerHandle **pHandle)
{
    delete (*pHandle)->pPlayer;
    delete *pHandle;
    *pHandle = nullptr;
}

StreamType CicadaSwitchStreamIndex(playerHandle *pHandle, int index)
{
    GET_PLAYER;

    if (player) {
        return player->SwitchStream(index);
    }

    return ST_TYPE_UNKNOWN;
}

int CicadaSetListener(playerHandle *pHandle, const playerListener &Listener)
{
    GET_PLAYER;

    if (player) {
        return player->SetListener(Listener);
    }

    return 0;
}

void CicadaSetOnRenderCallBack(playerHandle *pHandle, onRenderFrame cb, void *userData)
{
    GET_PLAYER;
    if (player) {
        player->SetOnRenderCallBack(cb, userData);
    }
}

void CicadaSetComponentCb(playerHandle *pHandle, player_component_type type, void *factory)
{
    GET_PLAYER;

    if (player) {
        switch (type) {
            case player_component_type_bit_demuxer:

            //    return player->setDemuxerFactory((Cicada::IDemuxerFactory *) factory);
            default:
                break;
        }
    }
}


void CicadaSetView(playerHandle *pHandle, void *view)
{
    GET_PLAYER;

    if (player) {
        return player->SetView(view);
    }
}


void CicadaSetDataSourceWithUrl(playerHandle *pHandle, const char *url)
{
    GET_PLAYER;

    if (player) {
        return player->SetDataSource(url);
    }
}


float CicadaGetVideoRenderFps(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->GetVideoRenderFps();
    }

    return 0;
}


float CicadaGetVideoDecodeFps(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->GetVideoDecodeFps();
    }

    return 0;
}

void CicadaAddCustomHttpHeader(playerHandle *pHandle, const char *header)
{
    GET_PLAYER;

    if (player) {
        return player->AddCustomHttpHeader(header);
    }
}


void CicadaRemoveAllCustomHttpHeader(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->RemoveAllCustomHttpHeader();
    }
}

void CicadaPreparePlayer(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->Prepare();
    }
}

void CicadaStartPlayer(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->Start();
    }
}


void CicadaPausePlayer(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->Pause();
//      float values[] = {0.5, 1.0, 1.5, 2.0,1.5,1.0};
//      static int index = 0;
//      player->setSpeed(values[index++ % (sizeof(values) / sizeof(values[0]))]);
    }
}

void CicadaPlayerSetSpeed(playerHandle *pHandle, float speed)
{
    GET_PLAYER;

    if (player) {
        player->setSpeed(speed);
    }
}

float CicadaPlayerGetSpeed(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->getSpeed();
    }

    return 0.0f;
}


void CicadaSeekToTime(playerHandle *pHandle, int64_t seekPos, bool bAccurate)
{
    GET_PLAYER;

    if (player) {
        return player->SeekTo(seekPos, bAccurate);
    }
}

int CicadaStopPlayer(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->Stop();
    }

    return 0;
}

int64_t CicadaGetMasterClockPts(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->GetMasterClockPts();
    }

    return 0;
}

void CicadaCaptureScreen(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->CaptureScreen();
    }
}

void CicadaSetVolume(playerHandle *pHandle, float volume)
{
    GET_PLAYER;

    if (player) {
        return player->SetVolume(volume);
    }
}

float CicadaGetVolume(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->GetVolume();
    }

    return 0;
}

PlayerStatus CicadaGetPlayerStatus(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->GetPlayerStatus();
    }

    return PLAYER_IDLE;
}

int64_t CicadaGetDuration(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->GetDuration();
    }

    return 0;
}

int64_t CicadaGetCurrentPosition(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->GetPlayingPosition();
    }

    return 0;
}

int64_t CicadaGetCurrentBufferedPosition(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->GetBufferPosition();
    }

    return 0;
}

void CicadaSetMute(playerHandle *pHandle, bool bMute)
{
    GET_PLAYER;

    if (player) {
        player->Mute(bMute);
    }
}

bool CicadaIsMute(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->IsMute();
    }

    return false;
}

void CicadaSetTimeout(playerHandle *pHandle, int timeout)
{
    GET_PLAYER;

    if (player) {
        player->SetTimeout(timeout);
    }
}

void CicadaSetDropBufferThreshold(playerHandle *pHandle, int dropValue)
{
    GET_PLAYER;

    if (player) {
        player->SetDropBufferThreshold(dropValue);
    }
}

void CicadaSetRefer(playerHandle *pHandle, const char *refer)
{
    GET_PLAYER;

    if (player) {
        player->SetRefer(refer);
    }
}


void CicadaSetUserAgent(playerHandle *pHandle, const char *userAgent)
{
    GET_PLAYER;

    if (player) {
        player->SetUserAgent(userAgent);
    }
}


void CicadaEnterBackGround(playerHandle *pHandle, bool back)
{
    GET_PLAYER;

    if (player) {
        player->EnterBackGround(back);
    }
}


void CicadaSetScaleMode(playerHandle *pHandle, ScaleMode mode)
{
    GET_PLAYER;

    if (player) {
        player->SetScaleMode(mode);
    }
}

ScaleMode CicadaGetScaleMode(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->GetScaleMode();
    }

    return ScaleMode::SM_EXTRACTTOFIT;
}


void CicadaSetRotateMode(playerHandle *pHandle, RotateMode mode)
{
    GET_PLAYER;

    if (player) {
        player->SetRotateMode(mode);
    }
}

RotateMode CicadaGetRotateMode(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->GetRotateMode();
    }

    return RotateMode::ROTATE_MODE_0;
}

void CicadaSetMirrorMode(playerHandle *pHandle, MirrorMode mode)
{
    GET_PLAYER;

    if (player) {
        player->SetMirrorMode(mode);
    }
}

void CicadaSetVideoBackgroundColor(playerHandle *pHandle, uint32_t color)
{
    GET_PLAYER;

    if (player) {
        player->SetVideoBackgroundColor(color);
    }
}

MirrorMode CicadaGetMirrorMode(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->GetMirrorMode();
    }

    return MirrorMode::MIRROR_MODE_NONE;
}


void CicadaSetLoop(playerHandle *pHandle, bool bLoop)
{
    GET_PLAYER;

    if (player) {
        player->SetLooping(bLoop);
    }
}

bool CicadaGetLoop(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->isLooping();
    }

    return false;
}

void CicadaSetAutoPlay(playerHandle *pHandle, bool bAutoPlay)
{
    GET_PLAYER;

    if (player) {
        player->SetAutoPlay(bAutoPlay);
    }
}

bool CicadaIsAutoPLay(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->IsAutoPlay();
    }

    return false;
}

void CicadaSetDecoderType(playerHandle *pHandle, DecoderType type)
{
    GET_PLAYER;

    if (player) {
        player->SetDecoderType(type);
    }
}

DecoderType CicadaGetDecoderType(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        return player->GetDecoderType();
    }

    return DT_SOFTWARE;
}

int CicadaGetCurrentStreamIndex(playerHandle *pHandle, StreamType type)
{
    GET_PLAYER;

    if (player) {
        return player->GetCurrentStreamIndex(type);
    }

    return -1;
}

StreamInfo *CicadaGetCurrentStreamInfo(playerHandle *pHandle, StreamType type)
{
    GET_PLAYER;

    if (player) {
        return player->GetCurrentStreamInfo(type);
    }

    return NULL;
}

void CicadaGetVideoResolution(playerHandle *pHandle, int &width, int &height)
{
    GET_PLAYER;

    if (player) {
        player->GetVideoResolution(width, height);
    }
}

void CicadaGetVideoRotation(playerHandle *pHandle, int &rotation)
{
    GET_PLAYER;

    if (player) {
        player->GetVideoRotation(rotation);
    }
}

std::string CicadaGetPropertyString(playerHandle *pHandle, PropertyKey key)
{
    GET_PLAYER;

    if (player) {
        return player->GetPropertyString(key);
    }

    return "";
}

int64_t CicadaGetPropertyLong(playerHandle *pHandle, PropertyKey key)
{
    GET_PLAYER;

    if (player) {
        return player->GetPropertyInt(key);
    }

    return 0;
}

int CicadaSetOption(playerHandle *pHandle, const char *key, const char *value)
{
    GET_PLAYER;

    if (player) {
        return player->SetOption(key, value);
    }

    return 0;
}

void CicadaGetOption(playerHandle *pHandle, const char *key, char *value)
{
    GET_PLAYER;

    if (player) {
        player->GetOption(key, value);
    }
}

void CicadaSetDefaultBandWidth(playerHandle *pHandle, int bandWidth)
{
    GET_PLAYER;

    if (player) {
        player->SetOption("bandWidth", AfString::to_string(bandWidth).c_str());
    }
}

void CicadaSetMediaFrameCb(playerHandle *pHandle, playerMediaFrameCb func, void *arg)
{
    GET_PLAYER;

    if (player) {
        return player->setMediaFrameCb(func, arg);
    }
}

int CicadaGetCurrentStreamMeta(playerHandle *pHandle, Stream_meta *meta, StreamType type)
{
    GET_PLAYER;

    if (player) {
        return player->getCurrentStreamMeta(meta, type);
    }

    return -EINVAL;
}

void CicadaReload(playerHandle *pHandle)
{
    GET_PLAYER;

    if (player) {
        player->reLoad();
    }
}

void CicadaSetErrorConverter(playerHandle *pHandle, ErrorConverter *converter)
{
    GET_PLAYER;

    if (player) {
        return player->setErrorConverter(converter);
    }
}

void CicadaSetBitStreamCb(playerHandle *pHandle, readCB read, seekCB seek, void *arg)
{
    GET_PLAYER;

    if (player) {
        return player->setBitStreamCb(read, seek, arg);
    }
}

void CicadaSetClockRefer(playerHandle *pHandle, clockRefer cb, void *arg)
{
    GET_PLAYER;
    if (player) {
        return player->setClockRefer(cb,arg);
    }
}

void CicadaAddExtSubtitle(playerHandle *pHandle, const char *uri)
{
    GET_PLAYER;

    if (player) {
        return player->addExtSubtitle(uri);
    }
}

void CicadaSelectExtSubtitle(playerHandle *pHandle, int index, bool select)
{
    GET_PLAYER;

    if (player) {
        player->selectExtSubtitle(index, select);
    }
}
