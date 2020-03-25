//
// Created by moqi on 2019-08-28.
//

#ifndef CICADA_PLAYER_VIDEO_TOOL_BOX_UTILS_H
#define CICADA_PLAYER_VIDEO_TOOL_BOX_UTILS_H

#include <VideoToolbox/VideoToolbox.h>
#include <utils/AFMediaType.h>

#ifdef __cplusplus
extern "C"{
#endif

typedef struct parserInfo_t {
    int width;
    int height;
} parserInfo;

#if 0
CFDictionaryRef videotoolbox_decoder_config_create(CMVideoCodecType codec_type, const uint8_t *pData, int size, parserInfo *pInfo);
#endif
CFDictionaryRef videotoolbox_buffer_attributes_create(int width, int height, OSType pix_fmt);

int parser_extradata(const uint8_t *pData, int size, parserInfo *pInfo, enum AFCodecID codecId);

#ifdef __cplusplus
}
#endif
#endif //CICADA_PLAYER_VIDEO_TOOL_BOX_UTILS_H
