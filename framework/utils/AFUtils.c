//
// Created by moqi on 2019-05-23.
//

#include "AFUtils.h"
#include <stdio.h>

#ifdef ANDROID
    #include "Android/cpu-features.h"
#elif defined __APPLE__

    #import <mach/host_info.h>
    #import <mach/mach_host.h>

#endif

int AFGetCpuCount()
{
#ifdef ANDROID
    return android_getCpuCount();
#elif defined __APPLE__
    host_basic_info_data_t hostInfo;
    mach_msg_type_number_t infoCount;
    infoCount = HOST_BASIC_INFO_COUNT;
    host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t) &hostInfo, &infoCount);
    return hostInfo.max_cpus;
#endif
    return 0;
}

static void handle_pipe(int sig)
{
//do nothing
}

void ignore_signal(int sig)
{
#ifndef _WIN32
    struct sigaction sa;
    sa.sa_handler = handle_pipe;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(sig, &sa, NULL);
#endif
}
