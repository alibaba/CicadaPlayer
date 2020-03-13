//
// Created by moqi on 2020/2/20.
//

#include "OpenSSAESDecrypt.h"

using namespace Cicada;

OpenSSAESDecrypt::OpenSSAESDecrypt() = default;

OpenSSAESDecrypt::~OpenSSAESDecrypt() = default;

int OpenSSAESDecrypt::setKey(const uint8_t *key, int key_bits)
{
    return AES_set_decrypt_key(key, 8 * AES_BLOCK_SIZE/*128*/, &mAesKey);
}

void OpenSSAESDecrypt::decrypt(uint8_t *dst, const uint8_t *src, int count, uint8_t *iv)
{
    AES_cbc_encrypt(src, dst, count * AES_BLOCK_SIZE, &mAesKey, iv, AES_DECRYPT);
}
