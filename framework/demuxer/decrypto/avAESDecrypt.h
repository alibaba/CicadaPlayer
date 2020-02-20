//
// Created by moqi on 2020/2/20.
//

#ifndef CICADAMEDIA_AVAESDECRYPT_H
#define CICADAMEDIA_AVAESDECRYPT_H

#include "IAESDecrypt.h"

namespace Cicada {
    class avAESDecrypt : public IAESDecrypt {
    public:
        avAESDecrypt();

        ~avAESDecrypt() override;

        int setKey(const uint8_t *key, int key_bits) override;

        void decrypt(uint8_t *dst, const uint8_t *src, int count, uint8_t *iv) override;

    private:
        struct AVAES *mAes{nullptr};

    };
}


#endif //CICADAMEDIA_AVAESDECRYPT_H
