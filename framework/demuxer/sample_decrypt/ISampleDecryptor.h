//
// Created by moqi on 2019/11/7.
//

#ifndef CICADAPLAYERSDK_ISAMPLEDECRYPTOR_H
#define CICADAPLAYERSDK_ISAMPLEDECRYPTOR_H

#include <utils/AFMediaType.h>

class ISampleDecryptor {

public:
    virtual int SetOption(const char *key, uint8_t *buffer, int size) = 0;

    virtual int decrypt(AFCodecID codecId, uint8_t *buffer, int size) = 0;

    virtual ~ISampleDecryptor() = default;

};


#endif //CICADAPLAYERSDK_ISAMPLEDECRYPTOR_H
