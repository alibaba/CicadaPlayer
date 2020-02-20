
//
// Created by moqi on 2018/10/23.
//
#define LOG_TAG "AES_128Decrypter"

#include <string>
#include <cstring>
#include "AES_128Decrypter.h"
#include "utils/frame_work_log.h"
#include <cerrno>
#include "../../decrypto/avAESDecrypt.h"

using namespace Cicada;

#define MIN(a, b) (((a)<(b))?(a):(b))

AES_128Decrypter::AES_128Decrypter(ISegDecrypter::read_cb read, void *arg) : ISegDecrypter(read, arg)
{
    mEos = false;
    mAESDecrypt = std::unique_ptr<Cicada::IAESDecrypt>(new avAESDecrypt());
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

    while (mInData - mInDataUsed < 2 * IAESDecrypt::BLOCK_SIZE) {
        int n = mReadCb(mReadCbArg, mInBuffer + mInData,
                        sizeof(mInBuffer) - mInData);

        if (n <= 0) {
            mEos = true;
            break;
        }

        mInData += n;
    }

    int blocks = (mInData - mInDataUsed) / IAESDecrypt::BLOCK_SIZE;

    if (!blocks) {
        mEos = true;
        return 0;
    }

    if (!mEos) {
        blocks--;
    }

    mAESDecrypt->decrypt(mOutBuffer, mInBuffer + mInDataUsed, blocks, mIvec);
    mOutData = IAESDecrypt::BLOCK_SIZE * blocks;
    mOutPtr = mOutBuffer;
    mInDataUsed += IAESDecrypt::BLOCK_SIZE * blocks;

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
    if (size != IAESDecrypt::BLOCK_SIZE) {
        return;
    }

    std::string skey = key;

    if (skey == "decryption key") {
        mValidKeyInfo = (mAESDecrypt->setKey(buffer, 8 * IAESDecrypt::BLOCK_SIZE) == 0);
    } else if (skey == "decryption IV") {
        memcpy(mIvec, buffer, IAESDecrypt::BLOCK_SIZE);
    }
}