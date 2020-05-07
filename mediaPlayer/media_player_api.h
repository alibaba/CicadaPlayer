#ifndef CICADA_PLAYER_H_
#define CICADA_PLAYER_H_

#include <vector>
#include <utils/AFMediaType.h>
#include <cacheModule/cache/CacheConfig.h>
#include "native_cicada_player_def.h"

//typedef struct Stream_meta_t Stream_meta;

typedef struct playerHandle_t playerHandle;

/*
 *create the Cicada player
 */
playerHandle *CicadaCreatePlayer();

/*
 * release Cicada player
 */
void CicadaReleasePlayer(playerHandle **player);

/*
 *set player listener
 */
int CicadaSetListener(playerHandle *pHandle, const playerListener &Listener);


void CicadaSetOnRenderCallBack(playerHandle *pHandle, onRenderFrame cb, void *userData);

/*
 * set external component callback
 */
attribute_deprecated
void CicadaSetComponentCb(playerHandle *pHandle, player_component_type type, void *factory);

/*
 * set bit stream data cb, if don't support seek, set it to null
 */
void CicadaSetBitStreamCb(playerHandle *pHandle, readCB read, seekCB seek, void *arg);

/*
 * set error code converter
 */
void CicadaSetErrorConverter(playerHandle *player, ErrorConverter *converter);

/*
 * set decoder type
 */
void CicadaSetDecoderType(playerHandle *player, DecoderType type);

/*
 * get decoder type
 */
DecoderType CicadaGetDecoderType(playerHandle *player);

/*
 *set player view
 */
void CicadaSetView(playerHandle *player, void *view);

/*
 *set play url source
 */
void CicadaSetDataSourceWithUrl(playerHandle *player, const char *url);

/*
 * switch stream by stream index
 */
StreamType CicadaSwitchStreamIndex(playerHandle *player, int index);

void CicadaAddCustomHttpHeader(playerHandle *pHandle, const char *header);

void CicadaRemoveAllCustomHttpHeader(playerHandle *pHandle);

void CicadaCaptureScreen(playerHandle *pHandle);

int64_t CicadaGetMasterClockPts(playerHandle *pHandle);

void CicadaSetClockRefer(playerHandle *pHandle,clockRefer cb, void *arg);

/*
* prepare to play
*/
void CicadaPreparePlayer(playerHandle *player);

/*
* start to play
*/
void CicadaStartPlayer(playerHandle *player);

/*
 * pause the player
 */
void CicadaPausePlayer(playerHandle *player);

/*
 * set playback speed, perhaps no effect if the range not be supported
 */
void CicadaPlayerSetSpeed(playerHandle *pHandle, float speed);

/*
 * get the playback speed
 */
float CicadaPlayerGetSpeed(playerHandle *pHandle);

/*
* Set the playback volume, 0-1
*/
void CicadaSetVolume(playerHandle *player, float volumne);

/**
 * Get the playback volume, 0-1
 */
float CicadaGetVolume(playerHandle *player);

/**
 * seek to the new position, Accurate or inAccurate
 */
void CicadaSeekToTime(playerHandle *player, int64_t seekPos, bool bAccurate);

/*
 * stop the playback
 */
int CicadaStopPlayer(playerHandle *player);

/*
 * get the duration
 */
int64_t CicadaGetDuration(playerHandle *player);

/*
 * get current stream index by stream type
 */
int CicadaGetCurrentStreamIndex(playerHandle *pHandle, StreamType type);

/*
 * get current stream info by stream type
 */
StreamInfo *CicadaGetCurrentStreamInfo(playerHandle *player, StreamType type);

/*
 * get the plaback current position
 */
int64_t CicadaGetCurrentPosition(playerHandle *player);

/*
 * get the playback buffered position
 */
int64_t CicadaGetCurrentBufferedPosition(playerHandle *player);

/*
 * enter background or enter forground, just for ios
 */
void CicadaEnterBackGround(playerHandle *player, bool back);

/*
 * set the playback mute
 */
void CicadaSetMute(playerHandle *player, bool bMute);

/**
 * get the playback muted or not
 */
bool CicadaIsMute(playerHandle *player);

/*
* set network timeout,defalut is 15000
*/
void CicadaSetTimeout(playerHandle *player, int timeout);

/*
 * set the max delay time when in live mode
 */
void CicadaSetDropBufferThreshold(playerHandle *player, int dropValue);

/*
 * set referer
 */
void CicadaSetRefer(playerHandle *player, const char *referer);

/*
 * set referer
 */
void CicadaSetUserAgent(playerHandle *player, const char *userAgent);

/*
 * set rending scaling mode
 */
void CicadaSetScaleMode(playerHandle *player, ScaleMode mode);

/**
 * get rending scaling mode
 */
ScaleMode CicadaGetScaleMode(playerHandle *player);

/**
 * set rending rotate mode .
 */
void CicadaSetRotateMode(playerHandle *pHandle, RotateMode mode);

/**
 * get rending rotate mode
 */
RotateMode CicadaGetRotateMode(playerHandle *player);

/**
 * set rending mirror mode .
 */
void CicadaSetMirrorMode(playerHandle *pHandle, MirrorMode mode);

/*
 * set clear color
 */
void CicadaSetVideoBackgroundColor(playerHandle *pHandle, uint32_t color);

/**
 * get rending mirror mode
 */
MirrorMode CicadaGetMirrorMode(playerHandle *player);

/*
 * set playback to loop mode
 */
void CicadaSetLoop(playerHandle *player, bool bLoop);

/*
 * get playback is loop or not
 */
bool CicadaGetLoop(playerHandle *pHandle);

/*
 * set playback to autoplay mode
 */
void CicadaSetAutoPlay(playerHandle *player, bool bAutoPlay);

/*
 * get playback is autoplay or not
 */
bool CicadaIsAutoPLay(playerHandle *pHandle);

void CicadaAddExtSubtitle(playerHandle *pHandle, const char *uri);

void CicadaSelectExtSubtitle(playerHandle *pHandle, int index, bool select);

/*
 * get video with and height
 */
void CicadaGetVideoResolution(playerHandle *player, int &width, int &height);

/*
 * get video rotation
 */
void CicadaGetVideoRotation(playerHandle *player, int &rotation);

/*
 * get string property
 */
std::string CicadaGetPropertyString(playerHandle *player, PropertyKey key);

/*
 * get long property
 */
int64_t CicadaGetPropertyLong(playerHandle *player, PropertyKey key);

/*
 * set option by key
 */
int CicadaSetOption(playerHandle *player, const char *key, const char *value);

/*
 * get option by key
 */
void CicadaGetOption(playerHandle *pHandle, const char *key, char *value);

void CicadaSetMediaFrameCb(playerHandle *player, playerMediaFrameCb func, void *arg);

int CicadaGetCurrentStreamMeta(playerHandle *player, Stream_meta *meta, StreamType type);

void CicadaReload(playerHandle *pHandle);

void CicadaSetDefaultBandWidth(playerHandle *player, int bandWidth);

#endif // CICADA_PLAYER_H_
