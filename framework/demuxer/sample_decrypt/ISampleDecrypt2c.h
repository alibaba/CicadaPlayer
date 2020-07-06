//
// Created by moqi on 2019/11/7.
//

#ifndef CICADA_PLAYER_ISAMPLEDECRYPT2C_H
#define CICADA_PLAYER_ISAMPLEDECRYPT2C_H

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <stdint.h>


int SampleDecryptDec(void *decryptor, enum AVCodecID codecId, uint8_t *buffer, int size);
#ifdef __cplusplus
}
#endif

#endif //CICADA_PLAYER_ISAMPLEDECRYPT2C_H
