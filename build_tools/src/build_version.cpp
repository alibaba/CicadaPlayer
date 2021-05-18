//
// Created by moqi on 2020/3/20.
//
#include <stdio.h>
#include "version.h"
#ifdef __ANDROID__
    #include <android/log.h>
#endif

extern "C" {
#include "include/libavcodec/avcodec.h"
#include "include/libavcodec/avfft.h"
#include "include/libavformat/avformat.h"
#include "include/libavutil/opt.h"
#include "include/libavutil/channel_layout.h"
#include "include/libavutil/common.h"
#include "include/libavutil/imgutils.h"
#include "include/libavutil/mathematics.h"
#include "include/libavutil/samplefmt.h"
#include "include/libavfilter/avfilter.h"
#include "include/libswresample/swresample.h"
#include "include/libavutil/avutil.h"
#include "include/libswscale/swscale.h"
}

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

int alivcffmpeg()
{
	avfilter_register_all();
	av_register_all();
	av_dump_format(nullptr, 0, nullptr, 0);
	return 0;
}
