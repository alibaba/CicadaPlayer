//
// Created by moqi on 2018/6/11.
//

#ifndef FRAMEWORK_FRAMEWORK_ERROR_H
#define FRAMEWORK_FRAMEWORK_ERROR_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int32_t error_type;
enum error_class {
    error_class_general = 0,
    error_class_network,
    error_class_codec,
    error_class_format,
    error_class_drm,

    error_class_internal = 0x10,
};


enum general_errno {
    general_errno_unknown = 200
};

enum network_errno {
    network_errno_general = 0,
    network_errno_unknown,
    network_errno_unsupported,
    network_errno_resolve,
    network_errno_connect_timeout,
    network_errno_could_not_connect,

    network_errno_http_400 = 100,
    network_errno_http_403,
    network_errno_http_404,
    network_errno_http_4xx,
    network_errno_http_5xx,
    network_errno_http_range = 120,

};

enum codec_error {
    codec_error_video_not_support,
    codec_error_audio_not_support,

    codec_error_video_device_error,
};

enum drm_error {
    drm_error_none = 0,
    drm_error_unsupport_scheme,
    drm_error_resource_busy,
    drm_error_key_response_null,
    drm_error_provision_response_null,
    drm_error_denied_by_server,
    drm_error_released,
    drm_error_provision_fail,

    drm_error_unknow = 99,
};

// TODO: SEGEND define here?
enum internal_errno {
    internal_errno_exit = 1, //interrupted by user
};

#define MKBETAG(a, b, c, d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))

static inline error_type gen_framework_errno(uint8_t eclass, uint8_t ferrno)
{
    return -(int) MKBETAG(0, 0, eclass, ferrno);
}

static inline uint8_t get_eclass(error_type err)
{
    return (uint8_t) ((-err & 0x0000FF00) >> 8);
}

static inline uint8_t get_errno(error_type err)
{
    return (uint8_t) (-err & 0x000000FF);
}

bool isHttpError(error_type err);

bool isLocalFileError(error_type err);

error_type gen_framework_http_errno(int httpCode);

const char *framework_err2_string(error_type err);


#define FRAMEWORK_ERR_EXIT         gen_framework_errno(error_class_internal,internal_errno_exit)

#define FRAMEWORK_NET_ERR_UNKNOWN  gen_framework_errno(error_class_network,network_errno_unknown)
#define FRAMEWORK_ERR_UNKNOWN      gen_framework_errno(error_class_general,general_errno_unknown)

#define FRAMEWORK_ERR_PROTOCOL_NOT_SUPPORT  gen_framework_errno(error_class_network, network_errno_unsupported)
#define FRAMEWORK_ERR_FORMAT_NOT_SUPPORT    gen_framework_errno(error_class_format,0)

#define FRAMEWORK_ERR(err) (-(err))

#ifdef __cplusplus
};
#endif

#endif //FRAMEWORK_FRAMEWORK_ERROR_H
