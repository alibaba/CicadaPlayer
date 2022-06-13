#ifndef NATIVE_CICADA_PLAYER_DEF_H
#define NATIVE_CICADA_PLAYER_DEF_H

#include <base/media/IAFPacket.h>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <memory>
#include <string>
#include <utils/CicadaType.h>

#define SELECT_TRACK_VIDEO_AUTO (-1)

const int MAX_OPT_VALUE_LENGTH = 256;

typedef enum _DecoderType {
    DT_HARDWARE = 0,
    DT_SOFTWARE,
} DecoderType;

typedef enum ScaleMode {
    SM_FIT = 0,
    SM_CROP,
    SM_EXTRACTTOFIT,
} ScaleMode;

typedef enum RotateMode {
    ROTATE_MODE_0 = 0,
    ROTATE_MODE_90 = 90,
    ROTATE_MODE_180 = 180,
    ROTATE_MODE_270 = 270,
} RotateMode;

typedef enum MirrorMode {
    MIRROR_MODE_NONE,
    MIRROR_MODE_HORIZONTAL,
    MIRROR_MODE_VERTICAL,
} MirrorMode;

typedef enum _StreamType {
    ST_TYPE_UNKNOWN = -1,
    ST_TYPE_VIDEO = 0,
    ST_TYPE_AUDIO,
    ST_TYPE_SUB,
} StreamType;

typedef enum _VideoHDRType {
    VideoHDRType_SDR,
    VideoHDRType_HDR10,
} VideoHDRType;

typedef enum _IpResolveType { IpResolveWhatEver, IpResolveV4, IpResolveV6 } IpResolveType;

#define VIDEO_FLAG 1 << ST_TYPE_VIDEO
#define AUDIO_FLAG 1 << ST_TYPE_AUDIO
#define SUB_FLAG 1 << ST_TYPE_SUB

typedef struct _StreamInfo {
    int streamIndex;
    StreamType type;
    char *description;

    int64_t bitrate;

    //video
    int videoBandwidth;
    int videoWidth;
    int videoHeight;
    VideoHDRType HDRType;

    //audio
    char *audioLang;
    int nChannels;
    int sampleRate;
    int sampleFormat;

    //subtitle
    char *subtitleLang;
} StreamInfo;

//apsara player callback define
typedef void (*playerVoidCallback)(void *userData);

typedef void (*playerType1Callback)(int64_t position, void *userData);

typedef void (*playerType12Callback)(int64_t width, int64_t height, void *userData);

typedef void (*playerType13Callback)(int64_t errorCode, const void *errorMsg, void *userData);

typedef void (*playerType123Callback)(int64_t index, int64_t size, const void *subtitle, void *userData);

typedef struct playerListener_t {
    playerVoidCallback LoopingStart;
    playerVoidCallback Prepared;
    playerVoidCallback Completion;
    playerVoidCallback FirstFrameShow;
    playerVoidCallback LoadingStart;
    playerVoidCallback LoadingEnd;
    playerVoidCallback AutoPlayStart;

    playerType1Callback Seeking;
    playerType1Callback SeekEnd;
    playerType1Callback PositionUpdate;
    playerType1Callback UtcTimeUpdate;
    playerType1Callback BufferPositionUpdate;
    playerType1Callback LoadingProgress;
    playerType1Callback CurrentDownLoadSpeed;

    playerType12Callback VideoSizeChanged;
    playerType12Callback StatusChanged;
    playerType12Callback VideoRendered;
    playerType12Callback AudioRendered;

    playerType13Callback ErrorCallback;

    playerType13Callback EventCallback;
    playerType13Callback MediaInfoGet;
    playerType13Callback StreamSwitchSuc;

    playerType123Callback CaptureScreen;
    playerType123Callback SubtitleHide;
    playerType123Callback SubtitleShow;
    playerType13Callback SubtitleExtAdd;
    playerType13Callback SubtitleHeader;
    void *userData;
} playerListener;

//apsara player status
typedef enum _PlayerStatus {
    PLAYER_IDLE = 0,
    PLAYER_INITIALZED,
    PLAYER_PREPARINIT,
    PLAYER_PREPARING,
    PLAYER_PREPARED,
    PLAYER_PLAYING,
    PLAYER_PAUSED,
    PLAYER_STOPPED,
    PLAYER_COMPLETION,
    PLAYER_ERROR = 99
} PlayerStatus;

enum player_component_type {
    player_component_type_bit_demuxer,
};


typedef enum SeekMode {
    SEEK_MODE_ACCURATE = 0x01,
    SEEK_MODE_INACCURATE = 0x10,
    //to be defined,
} SeekMode;

typedef enum PropertyKey {
    PROPERTY_KEY_RESPONSE_INFO = 0,
    PROPERTY_KEY_CONNECT_INFO = 1,
    PROPERTY_KEY_OPEN_TIME_STR = 2,
    PROPERTY_KEY_PROBE_STR = 3,
    PROPERTY_KEY_VIDEO_BUFFER_LEN = 4,
    PROPERTY_KEY_DELAY_INFO = 5,
    PROPERTY_KEY_REMAIN_LIVE_SEG = 6,
    PROPERTY_KEY_NETWORK_IS_CONNECTED = 7,
    PROPERTY_KEY_PLAY_CONFIG = 8,
    PROPERTY_KEY_DECODE_INFO = 9,
    PROPERTY_KEY_HLS_KEY_URL = 10,
    PROPERTY_KEY_RE_BUFFERING,
    PROPERTY_KEY_DOWNLOAD_COMPLETED,
    PROPERTY_KEY_VIDEO_DROPPED_INFO,
    PROPERTY_KEY_NETWORK_SPEED,
    PROPERTY_KEY_BUFFER_INFO,
    PROPERTY_KEY_NETWORK_REQUEST_LIST,
    PROPERTY_KEY_RENDER_INFO,
    PROPERTY_KEY_CONTAINER_INFO,
} PropertyKey;

typedef enum VideoTag {
    VIDEO_TAG_NONE = 0,
    VIDEO_TAG_SDR = 1 << 0,
    VIDEO_TAG_HDR10 = 1 << 1,
    VIDEO_TAG_WIDEVINE_L1 = 1 << 2,
    VIDEO_TAG_WIDEVINE_L3 = 1 << 3,
    VIDEO_TAG_FAIRPLAY = 1 << 4,
} VideoTag;

class AMediaFrame;

typedef void (*playerMediaFrameCb)(void *arg, const IAFPacket *frame, StreamType type);

typedef int (*readCB)(void *arg, uint8_t *buffer, int size);

typedef int64_t (*seekCB)(void *arg, int64_t offset, int whence);

typedef int64_t(*clockRefer)(void *arg);

typedef bool (*onRenderFrame)(void *userData, IAFFrame *frame);

typedef bool (*UpdateViewCB)(int videoType, void *userData);

typedef std::string (*UrlHashCB)(const char *url, void *userData);

class ErrorConverter {
public:
    virtual int ConvertErrorCode(int code, int &outCode, std::string &outStr) = 0;
};

class MediaInfo {
public:
    int64_t totalBitrate{0};

    std::deque<StreamInfo *> mStreamInfoQueue{};
};
#define EXT_STREAM_BASE (1<<10)

#endif // NATIVE_CICADA_PLAYER_DEF_H
