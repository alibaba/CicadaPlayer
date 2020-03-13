//
// Created by moqi on 2020/2/20.
//

#ifndef CICADAMEDIA_OPENSSAESDECRYPT_H
#define CICADAMEDIA_OPENSSAESDECRYPT_H

#include <openssl/aes.h>
#include "IAESDecrypt.h"

namespace Cicada {
    class OpenSSAESDecrypt : public IAESDecrypt {
    public:
        OpenSSAESDecrypt();

        ~OpenSSAESDecrypt() override;

        int setKey(const uint8_t *key, int key_bits) override;

        void decrypt(uint8_t *dst, const uint8_t *src, int count, uint8_t *iv) override;

    private:
        AES_KEY mAesKey{};

    };
}


#endif //CICADAMEDIA_OPENSSAESDECRYPT_H
