//
// Created by moqi on 2020/2/20.
//
extern "C" {
#include <libavutil/mem.h>
#include <libavutil/aes.h>
}

#include "avAESDecrypt.h"

using namespace Cicada;

avAESDecrypt::avAESDecrypt()
{
    mAes = av_aes_alloc();
}

avAESDecrypt::~avAESDecrypt()
{
    av_free(mAes);
}

void avAESDecrypt::decrypt(uint8_t *dst, const uint8_t *src, int count, uint8_t *iv)
{
    av_aes_crypt(mAes, dst, src, count, iv, 1);
}

int avAESDecrypt::setKey(const uint8_t *key, int key_bits)
{
    return av_aes_init(mAes, key, key_bits, 1);
}


avAESEncrypt::avAESEncrypt()
{
    mAes = av_aes_alloc();
}

avAESEncrypt::~avAESEncrypt()
{
    av_free(mAes);
}

int avAESEncrypt::setKey(const uint8_t *key, int key_bits)
{
    return av_aes_init(mAes, key, key_bits, 0);
}

void avAESEncrypt::encrypt(uint8_t *dst, const uint8_t *src, int count, uint8_t *iv)
{
    av_aes_crypt(mAes, dst, src, count, iv, 0);
}
