//
// Created by moqi on 2018/10/23.
//

#ifndef CICADA_PLAYER_SEGDECRYPTORFACTORY_H
#define CICADA_PLAYER_SEGDECRYPTORFACTORY_H

#include "ISegDecrypter.h"
#include "SegmentEncryption.h"
#include <map>

using namespace std;

class SegDecryptorFactory {

public:
    static SegDecryptorFactory *getInstance();

    static ISegDecrypter *create(SegmentEncryption::encryption_method method, ISegDecrypter::read_cb read, void *arg);


private:
    SegDecryptorFactory() = default;
    ~SegDecryptorFactory() = default;

private:
    static SegDecryptorFactory sInstance;
};

#endif //CICADA_PLAYER_SEGDECRYPTORFACTORY_H
