//
// Created by moqi on 2018/6/11.
//
#include <utils/frame_work_log.h>
#include <cerrno>
#include "framework_error.h"

int main()
{
    error_type err = gen_framework_errno(error_class_network, network_errno_http_403);
    AF_LOGD("err 0x%04x is %s\n", -err, framework_err2_string(err));
    err = gen_framework_errno(error_class_general, EAGAIN);
    AF_LOGD("err 0x%04x is %s\n", -err, framework_err2_string(err));
    AF_LOGD("%s", framework_err2_string(FRAMEWORK_NET_ERR_UNKNOWN));
    return 0;
}