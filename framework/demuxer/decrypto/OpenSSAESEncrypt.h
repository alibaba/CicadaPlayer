//
// Created by lifujun on 2020/3/13.
//

#ifndef SOURCE_OPENSSAESENCRYPT_H
#define SOURCE_OPENSSAESENCRYPT_H

#include <openssl/aes.h>
#include "IAESDecrypt.h"

namespace Cicada {
    class OpenSSAESEncrypt : public IAESEncrypt{
    public:
        OpenSSAESEncrypt();

        ~OpenSSAESEncrypt() override;

        int setKey(const uint8_t *key, int key_bits) override;

        void encrypt(uint8_t *dst, const uint8_t *src, int count, uint8_t *iv) override;

    private:
        AES_KEY mAesKey{};
    };
}


#endif //SOURCE_OPENSSAESENCRYPT_H
