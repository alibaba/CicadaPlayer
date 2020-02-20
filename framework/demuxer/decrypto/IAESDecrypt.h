//
// Created by moqi on 2020/2/20.
//

#ifndef CICADAMEDIA_IAESDECRYPT_H
#define CICADAMEDIA_IAESDECRYPT_H

#include <cstdint>


namespace Cicada {
    class IAESDecrypt {
    public:
        const static int BLOCK_SIZE = 16;

        IAESDecrypt() = default;

        virtual ~IAESDecrypt() = default;

        virtual int setKey(const uint8_t *key, int key_bits) = 0;

        virtual void decrypt(uint8_t *dst, const uint8_t *src, int count, uint8_t *iv) = 0;

    };
}


#endif //CICADAMEDIA_IAESDECRYPT_H
