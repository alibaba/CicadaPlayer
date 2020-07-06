//
// Created by moqi on 2019/11/7.
//

#include "ISampleDecryptor.h"
#include <utils/ffmpeg_utils.h>
#include "ISampleDecrypt2c.h"

int SampleDecryptDec(void *decryptor, enum AVCodecID codecId, uint8_t *buffer, int size)
{
    return ((ISampleDecryptor *) (decryptor))->decrypt(AVCodec2CicadaCodec(codecId), buffer, size);
}