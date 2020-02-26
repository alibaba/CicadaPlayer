//
// Created by moqi on 2018/2/1.
//

#ifndef FRAMEWORK_FRAMEWORK_TYPE_H
#define FRAMEWORK_FRAMEWORK_TYPE_H

#include <cstdint>

#ifndef WIN32

#include <unistd.h>

#endif

typedef int (*demuxer_callback_read)(void *arg, uint8_t *buffer, int size);

typedef int64_t (*demuxer_callback_seek)(void *arg, int64_t offset, int whence);

typedef void (*demuxer_callback_interrupt_data)(void *arg, int inter);

typedef int (*demuxer_callback_open)(void *arg, const char *url, int64_t start, int64_t end);

enum {
    EN_FRAMEWORK_SERVICE_PLAYER_BASE = 0x00001000,
    EN_FRAMEWORK_SERVICE_PLAYER_DATA_SOURCE_SERVICE,
    EN_FRAMEWORK_SERVICE_PLAYER_DATA_SOURCE_PROXY_SERVICE,
    EN_FRAMEWORK_SERVICE_PLAYER_DEMUXER_SERVICE,
    EN_FRAMEWORK_SERVICE_PLAYER_DEMUXER_PROXY_SERVICE,
};

#endif //FRAMEWORK_FRAMEWORK_TYPE_H
