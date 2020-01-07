//
// Created by moqi on 2018/5/12.
//

#include "err.h"

extern "C"{
    #include "libavutil/error.h"
}

int get_ffmpeg_error_message(int err,char* errMsg,int size) {
    int ret = av_strerror(err, errMsg, size);
    return ret;
}
