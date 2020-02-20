//
// Created by moqi on 2018/10/23.
//

#ifndef CICADA_PLAYER_AES_128DECRYPTORT_H
#define CICADA_PLAYER_AES_128DECRYPTORT_H


#include "ISegDecrypter.h"
#include "../../decrypto/IAESDecrypt.h"
#include <memory>

class AES_128Decrypter : public ISegDecrypter {
public:
    AES_128Decrypter(read_cb read, void *arg);

    ~AES_128Decrypter() override;

    int Read(uint8_t *buffer, int size) override;

    void SetOption(const char *key, uint8_t *buffer, int size) override;

    void flush() override
    {
        mEos = false;
        mOutPtr = nullptr;
        mInData = 0;
        mInDataUsed = 0;
        mOutData = 0;
    };

protected:
    static const int MAX_BUFFER_BLOCKS = 257;
    uint8_t mIvec[Cicada::IAESDecrypt::BLOCK_SIZE]{0};
    bool mEos;
    bool mValidKeyInfo{false};
    uint8_t *mOutPtr = nullptr;
    int mInData = 0, mInDataUsed = 0, mOutData = 0;

    uint8_t mInBuffer[Cicada::IAESDecrypt::BLOCK_SIZE * MAX_BUFFER_BLOCKS]{},
            mOutBuffer[Cicada::IAESDecrypt::BLOCK_SIZE * MAX_BUFFER_BLOCKS]{};

    std::unique_ptr<Cicada::IAESDecrypt> mAESDecrypt{nullptr};


};


#endif //CICADA_PLAYER_AES_128DECRYPTORT_H
