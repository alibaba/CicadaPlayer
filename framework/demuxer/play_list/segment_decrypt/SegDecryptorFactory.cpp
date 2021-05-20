//
// Created by moqi on 2018/10/23.
//

#include <mutex>
#include "SegDecryptorFactory.h"
#include "AES_128Decrypter.h"
#include "ISegDecryptorPrototype.h"


static SegDecryptorFactory *sInstance = nullptr;

ISegDecrypter *SegDecryptorFactory::create(SegmentEncryption::encryption_method method,
        ISegDecrypter::read_cb read, void *arg)
{
    switch (method) {
        case SegmentEncryption::AES_128 :
            return new AES_128Decrypter(read, arg);

        default:
            return ISegDecryptorPrototype::create(method, read, arg);
    }
}

SegDecryptorFactory *SegDecryptorFactory::getInstance(void)
{
    static std::once_flag oc;
    std::call_once(oc, [&] {
        sInstance = new SegDecryptorFactory();
    });
    return sInstance;
}
