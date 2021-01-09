//
// Created by moqi on 2018/5/10.
//

#ifndef FRAMEWORK_LOG_H
#define FRAMEWORK_LOG_H

#include <af_config.h>

#include "CicadaType.h"
#include <inttypes.h>
#include <stdint.h>

#define AF_LOG_LEVEL_NONE  0
#define AF_LOG_LEVEL_FATAL  8
#define AF_LOG_LEVEL_ERROR  16
#define AF_LOG_LEVEL_WARNING  24
#define AF_LOG_LEVEL_INFO  32
#define AF_LOG_LEVEL_DEBUG  48
#define AF_LOG_LEVEL_TRACE  56


#ifndef LOG_TAG
#define LOG_TAG ""
#endif

//typedef void (*log_callback)(char levelChar , char* tag , char* msg , void* userData);

typedef void(*log_back)(void *arg, int prio, const char *buf);
//#define printf "can't use printf,use LE_LOG* please"
#ifdef __cplusplus
extern "C" {
#endif

CICADA_EXTERN int __log_print(int prio, const char *tag, const char *fmt, ...);

void log_set_enable_console(int enable);

void log_set_log_level(int level);

void log_set_level(int level, int enable_console);

int log_get_level();

void log_set_back(log_back func, void *arg);

void log_enable_color(int enable);

char *getVersion();

char *getTime();

#ifdef __cplusplus
}
#endif

#ifndef NOLOGI
#define AF_LOGI(...) __log_print(AF_LOG_LEVEL_INFO,LOG_TAG,__VA_ARGS__)
#else
#define AF_LOGI(...)
#endif
#ifndef NOLOGD
#define AF_LOGD(...) __log_print(AF_LOG_LEVEL_DEBUG,LOG_TAG,__VA_ARGS__)
#else
#define AF_LOGD(...)
#endif
#ifndef NOLOGW
#define AF_LOGW(...) __log_print(AF_LOG_LEVEL_WARNING,LOG_TAG,__VA_ARGS__)
#else
#define AF_LOGW(...)
#endif
#define AF_LOGE(...) __log_print(AF_LOG_LEVEL_ERROR,LOG_TAG,__VA_ARGS__)
#define AF_LOGF(...) __log_print(LOG_LEVEL_FATAL,LOG_TAG,__VA_ARGS__)

#define AF_TRACE do { AF_LOGD("%s:%d(%s)\n",__FILE__,__LINE__,__func__);} while(0)

#define AF_DUMP_INT(LINE) AF_LOGD("%s is %lld\n",#LINE,LINE)

#ifdef __cplusplus
extern "C" {
#endif
#ifndef NOLOGD

static inline void _hex_dump(const uint8_t *pBuffer, int size)
{
    if(pBuffer) {
        int i;
        for (i = 0; i < size; i++)
            AF_LOGD("%02x ", pBuffer[i]);
        AF_LOGD("\n");
    }
}
#else
static inline void _hex_dump(uint8_t *pBuffer, int size)
{
}

#endif

#ifdef __cplusplus
}
#endif

#endif //FRAMEWORK_UTILS_H
