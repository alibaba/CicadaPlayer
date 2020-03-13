//
// Created by lifujun on 2020/3/13.
//

#include "OpenSSAESEncrypt.h"

using namespace Cicada ;
OpenSSAESEncrypt::OpenSSAESEncrypt() = default;

OpenSSAESEncrypt::~OpenSSAESEncrypt() = default;

int OpenSSAESEncrypt::setKey(const uint8_t *key, int key_bits)
{
    return AES_set_decrypt_key(key, 8 * AES_BLOCK_SIZE/*128*/, &mAesKey);
}

void OpenSSAESEncrypt::encrypt(uint8_t *dst, const uint8_t *src, int count, uint8_t *iv)
{
    AES_cbc_encrypt(src, dst, count * AES_BLOCK_SIZE, &mAesKey, iv, AES_ENCRYPT);
}

