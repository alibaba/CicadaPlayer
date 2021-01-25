//
// Created by moqi on 2018/6/11.
//

#include "framework_error.h"
#include "../frame_work_log.h"
#include <errno.h>
#include <string.h>

const char *network_err2_string(uint8_t Errno)
{
    switch (Errno) {
        case network_errno_unsupported:
            return "Unsupported protocol";

        case network_errno_connect_timeout:
            return "Connection Timeout was reached";

        case network_errno_resolve:
            return "Couldn't resolve host name";

        case network_errno_could_not_connect:
            return "Couldn't connect to server";

        case network_errno_http_403:
            return "Server returned 403 Forbidden (access denied)";

        case network_errno_http_400:
            return "Server returned 400 Bad Request";

        case network_errno_http_404:
            return "Server returned 404 Not Found";

        case network_errno_http_4xx:
            return "Server returned 4XX Client Error, but not one of 40{0,1,3,4}";

        case network_errno_http_5xx:
            return "Server returned 5XX Server Error reply";

        case network_errno_http_range:
            return "Requested range was not delivered by the server";

        default:
            return "Unspecific network error";
    }
}

const char *codec_err2_string(uint8_t Errno)
{
    switch (Errno) {
        case codec_error_video_not_support:
            return "video codec not support";

        case codec_error_audio_not_support:
            return "audio codec not support";

        case codec_error_video_device_error:
            return "video decoder open error";

        default:
            return "Unknown codec error";
    }
}

const char *drm_err2_string(uint8_t Errno)
{
    switch (Errno) {
        case drm_error_denied_by_server:
            return "denied by server";
        case drm_error_key_response_null:
            return "key response is null";
        case drm_error_provision_response_null:
            return "provision response is null";
        case drm_error_resource_busy:
            return "resource busy";
        case drm_error_unsupport_scheme:
            return "unsupport scheme";
        case drm_error_released:
            return "drm released";
        case drm_error_provision_fail:
            return "drm provision fail";
        case drm_error_unknow:
            return "unknow drm error";
        default:
            return "Unknown drm error";
    }
}

const char *strerror_ext(uint8_t Errno)
{
    if (Errno < 200)
        return strerror(Errno);

    switch (Errno) {
        default:
        case general_errno_unknown:
            return "Unknown Error";
    }
}

const char *internal_err2_string(uint8_t Errno)
{
    switch (Errno) {
        case internal_errno_exit:
            return "Immediate exit requested";

        default:
            return "Unknown Error";
    }
}

const char *framework_err2_string(error_type err)
{
    if (err >= 0) {
        return "Success";
    }

    uint8_t eclass = get_eclass(err);
    uint8_t Errno = get_errno(err);
    //   AF_LOGD("errno is %d\n",errno);

    switch (eclass) {
        case error_class_general:
            return strerror_ext(Errno);

        case error_class_network:
            return network_err2_string(Errno);

        case error_class_codec:
            return codec_err2_string(Errno);

        case error_class_format:
            return "format not support";

        case error_class_internal:
            return internal_err2_string(Errno);

        case error_class_drm:
            return drm_err2_string(Errno);

        default:
            return "Unknown Error";
    }
}

bool isHttpError(error_type err)
{
    if (err >= 0) {
        return false;
    }

    uint8_t eclass = get_eclass(err);

    if (eclass == error_class_network) {
        uint8_t Errno = get_errno(err);

        if (Errno >= network_errno_http_400 && Errno <= network_errno_http_range) {
            return true;
        }
    }

    return false;
}

bool isLocalFileError(error_type err)
{
    return err == -ENOENT || err == -EACCES;
}

error_type gen_framework_http_errno(int httpCode)
{
    uint8_t errorno = 0;

    if (httpCode < 400 || httpCode >= 600) {
        return 0;
    }

    if (httpCode == 400)
        errorno = network_errno_http_400;
    else if (httpCode == 403)
        errorno = network_errno_http_403;
    else if (httpCode == 404)
        errorno = network_errno_http_404;
    else if (httpCode < 500)
        errorno = network_errno_http_4xx;
    else
        errorno = network_errno_http_5xx;

    return gen_framework_errno(error_class_network, errorno);
}
