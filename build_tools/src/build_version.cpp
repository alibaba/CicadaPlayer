//
// Created by moqi on 2020/3/20.
//
#include <stdio.h>
#include "version.h"
#ifdef __ANDROID__
    #include <android/log.h>
#endif

static const char *get_external_build_version();

static const char *v = get_external_build_version();

static const char *get_external_build_version()
{
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_VERBOSE, "version", "external version is %s\n", build_version);
#else
    printf("external version is %s\n", build_version);
#endif
    return build_version;
}

