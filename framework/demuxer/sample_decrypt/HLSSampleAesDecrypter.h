//
// Created by moqi on 2018/10/23.
//

#ifndef CICADA_PLAYER_ISAMPLEAESDECRYPTER_H
#define CICADA_PLAYER_ISAMPLEAESDECRYPTER_H


#include <cstdint>
#include <openssl/aes.h>
#include "ISampleDecryptor.h"

class HLSSampleAesDecrypter : public ISampleDecryptor {

public:
    HLSSampleAesDecrypter()
    {
    }

    ~HLSSampleAesDecrypter() override;

    int SetOption(const char *key, uint8_t *buffer, int size) override;

    int decrypt(AFCodecID codecId, uint8_t *buffer, int size) override;

private:
    int decryptH264Video(uint8_t *buffer, int size);

    int decryptAACAudio(uint8_t *buffer, int size);

private:
    void decrypt_nalunit(uint8_t *nal_unit, int nal_size);

private:
    static const int VIDEO_CLEAR_LEAD = 32;
    static const int AUDIO_CLEAR_LEAD = 16;
    uint8_t mIvec[AES_BLOCK_SIZE]{0};
    AES_KEY mAesKey;
    bool mValidKeyInfo{false};
};


#endif //CICADA_PLAYER_ISAMPLEAESDECRYPTER_H
