//
// Created by moqi on 2019/9/26.
//

#ifndef CICADA_PLAYER_CODECPROTOTYPE_H
#define CICADA_PLAYER_CODECPROTOTYPE_H
#include <memory>
#include "IDecoder.h"

class codecPrototype {
//    static vector<codecPrototype *> codecQueue;
    static codecPrototype*  codecQueue [10];
    static int _nextSlot;
public:
    virtual ~codecPrototype() = default;

    virtual Cicada::IDecoder *clone() = 0;

    virtual bool is_supported(AFCodecID code, uint64_t flags, int maxSize) = 0;

    static void addPrototype(codecPrototype *se);

    static Cicada::IDecoder *create(AFCodecID code, uint64_t flags, int maxSize);

};


#endif //CICADA_PLAYER_CODECPROTOTYPE_H
