//
// Created by moqi on 2018/5/12.
//

#ifndef FRAMEWORK_ERR_H
#define FRAMEWORK_ERR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    int get_ffmpeg_error_message(int err,char* errMsg,int size);


#ifdef __cplusplus
};
#endif

#endif //FRAMEWORK_ERR_H
