//
// Created by lifujun on 2019/12/25.
//

#ifndef SOURCE_ISEGDECRYPTORPROTOTYPE_H
#define SOURCE_ISEGDECRYPTORPROTOTYPE_H


#include "SegmentEncryption.h"
#include "ISegDecrypter.h"

class ISegDecryptorPrototype {
    static ISegDecryptorPrototype *decryptorPrototypeQueue[10];
    static int _nextSlot;
public:
    virtual ~ISegDecryptorPrototype() = default;

    static ISegDecrypter* create(SegmentEncryption::encryption_method method, ISegDecrypter::read_cb read, void *arg);

protected:
    static void addPrototype(ISegDecryptorPrototype *se);

private:
    virtual ISegDecrypter *clone(SegmentEncryption::encryption_method method, ISegDecrypter::read_cb read, void *arg) = 0;

    virtual bool is_supported(SegmentEncryption::encryption_method method , ISegDecrypter::read_cb read) = 0;

};


#endif //SOURCE_ISEGDECRYPTORPROTOTYPE_H
