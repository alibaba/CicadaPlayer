//
// Created by lifujun on 2019/1/4.
//

#ifndef SOURCE_MEDIA_PLAYER_ERROR_MAP_H
#define SOURCE_MEDIA_PLAYER_ERROR_MAP_H


#include <utils/errors/framework_error.h>
#include "media_player_error_def.h"

using namespace Cicada;

static int strerror2_code(uint8_t Errno);
static int network_err2_code(uint8_t Errno);

static int codec_err2_code(uint8_t Errno);

static int internal_err2_code(uint8_t Errno);

static int framework_error2_code(error_type err)
{
    if (err >= 0) {
        return 0;
    }
    uint8_t eclass = get_eclass(err);
    uint8_t Errno = get_errno(err);
    //   AF_LOGD("errno is %d\n",errno);

    switch (eclass) {
        case error_class_general:
            return strerror2_code(Errno);
        case error_class_network:
            return network_err2_code(Errno);
        case error_class_codec:
            return codec_err2_code(Errno);
        case error_class_internal:
            return internal_err2_code(Errno);
        default:
            return MEDIA_PLAYER_ERROR_UNKNOWN;
    }
}

static int internal_err2_code(uint8_t Errno)
{
    switch (Errno) {
        case internal_errno_exit:
            return MEDIA_PLAYER_ERROR_INERNAL_EXIT;
        default:
            return MEDIA_PLAYER_ERROR_INERNAL_UNKNOWN;
    }
}

static int codec_err2_code(uint8_t Errno)
{
    switch (Errno){
        case codec_error_video_not_support:
            return MEDIA_PLAYER_ERROR_CODEC_VIDEO_NOT_SUPPORT;
        case codec_error_audio_not_support:
            return MEDIA_PLAYER_ERROR_CODEC_AUDIO_NOT_SUPPORT;
        case codec_error_video_device_error:
            return MEDIA_PLAYER_ERROR_CODEC_VIDEO_OPEN_ERROR;
        default:
            return MEDIA_PLAYER_ERROR_CODEC_UNKNOWN;
    }
}

static int network_err2_code(uint8_t Errno)
{
    switch (Errno) {
        case network_errno_unsupported:
            return MEDIA_PLAYER_ERROR_NETWORK_UNSUPPORTED;
        case network_errno_connect_timeout:
            return MEDIA_PLAYER_ERROR_NETWORK_CONNECT_TIMEOUT;
        case network_errno_resolve:
            return MEDIA_PLAYER_ERROR_NETWORK_RESOLVE;
        case network_errno_could_not_connect:
            return MEDIA_PLAYER_ERROR_NETWORK_COULD_NOT_CONNECT;
        case network_errno_http_403:
            return MEDIA_PLAYER_ERROR_NETWORK_HTTP_403;
        case network_errno_http_404:
            return MEDIA_PLAYER_ERROR_NETWORK_HTTP_404;
        case network_errno_http_4xx:
            return MEDIA_PLAYER_ERROR_NETWORK_HTTP_4XX;
        case network_errno_http_5xx:
            return MEDIA_PLAYER_ERROR_NETWORK_HTTP_5XX;
        case network_errno_http_range:
            return MEDIA_PLAYER_ERROR_NETWORK_HTTP_RANGE;
        default:
            return MEDIA_PLAYER_ERROR_NETWORK_UNKNOWN;
    }
}

static int strerror2_code(uint8_t Errno)
{
    if (Errno < 200) {
        return MEDIA_PLAYER_ERROR_GENERAL_UNKNOWN + Errno;
    }

    return MEDIA_PLAYER_ERROR_UNKNOWN;
}

#endif //SOURCE_MEDIA_PLAYER_ERROR_MAP_H
