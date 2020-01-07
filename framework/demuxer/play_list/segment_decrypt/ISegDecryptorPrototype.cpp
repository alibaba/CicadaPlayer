//
// Created by lifujun on 2019/12/25.
//

#include "ISegDecryptorPrototype.h"

ISegDecryptorPrototype *ISegDecryptorPrototype::decryptorPrototypeQueue[];
int ISegDecryptorPrototype::_nextSlot;

ISegDecrypter *ISegDecryptorPrototype::create(SegmentEncryption::encryption_method method,
        ISegDecrypter::read_cb read, void *arg)
{
    for (int i = 0; i < _nextSlot; i++) {
        if (decryptorPrototypeQueue[i]->is_supported(method, read)) {
            return decryptorPrototypeQueue[i]->clone(method, read, arg);
        }
    }

    return nullptr;
}

void ISegDecryptorPrototype::addPrototype(ISegDecryptorPrototype *se)
{
    decryptorPrototypeQueue[_nextSlot++] = se;
}
