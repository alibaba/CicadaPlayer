//
// Created by moqi on 2018/10/23.
//
#define LOG_TAG "AES_128Decrypter"

#include <string>
#include <demuxer/demuxer_service.h>
#include "AES_128Decrypter.h"
#include "utils/frame_work_log.h"
#include <cerrno>

#define MIN(a, b) (((a)<(b))?(a):(b))

AES_128Decrypter::AES_128Decrypter(ISegDecrypter::read_cb read, void *arg) : ISegDecrypter(read, arg)
{
    mEos = false;
}

AES_128Decrypter::~AES_128Decrypter() = default;

int AES_128Decrypter::Read(uint8_t *buffer, int size)
{
    if (!mValidKeyInfo || mReadCb == nullptr) {
        return -EINVAL;
    }

retry:

    if (mOutData > 0) {
        size = std::min(size, mOutData);
        memcpy(buffer, mOutPtr, size);
        mOutPtr += size;
        mOutData -= size;
//        position = c->position + size;
        return size;
    }

    while (mInData - mInDataUsed < 2 * AES_BLOCK_SIZE) {
        int n = mReadCb(mReadCbArg, mInBuffer + mInData,
                        sizeof(mInBuffer) - mInData);

        if (n <= 0) {
            mEos = true;
            break;
        }

        mInData += n;
    }

    int blocks = (mInData - mInDataUsed) / AES_BLOCK_SIZE;

    if (!blocks) {
        mEos = true;
        return 0;
    }

    if (!mEos) {
        blocks--;
    }

    AES_cbc_encrypt(mInBuffer + mInDataUsed, mOutBuffer, blocks * AES_BLOCK_SIZE, &mAesKey, mIvec, AES_DECRYPT);
    mOutData = AES_BLOCK_SIZE * blocks;
    mOutPtr = mOutBuffer;
    mInDataUsed += AES_BLOCK_SIZE * blocks;

    if (mInDataUsed >= sizeof(mInBuffer) / 2) {
        memmove(mInBuffer, mInBuffer + mInDataUsed,
                mInData - mInDataUsed);
        mInData -= mInDataUsed;
        mInDataUsed = 0;
    }

    if (mEos) {
        // Remove PKCS7 padding at the end
        int padding = mOutBuffer[mOutData - 1];
        mOutData -= padding;
    }

    goto retry;
}

void AES_128Decrypter::SetOption(const char *key, uint8_t *buffer, int size)
{
    if (size != AES_BLOCK_SIZE) {
        return;
    }

    std::string skey = key;

    if (skey == "decryption key") {
        mValidKeyInfo = (AES_set_decrypt_key(buffer, 8 * AES_BLOCK_SIZE/*128*/, &mAesKey) == 0);
    } else if (skey == "decryption IV") {
        memcpy(mIvec, buffer, AES_BLOCK_SIZE);
    }
}