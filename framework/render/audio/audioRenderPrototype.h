//
// Created by moqi on 2019/10/21.
//

#ifndef CICADA_PLAYER_AUDIORENDERPROTOTYPE_H
#define CICADA_PLAYER_AUDIORENDERPROTOTYPE_H

#include "IAudioRender.h"
#include <memory>

class audioRenderPrototype {
    static audioRenderPrototype *renderQueue[10];
    static int _nextSlot;
public:
    virtual ~audioRenderPrototype() = default;

    virtual Cicada::IAudioRender *clone() = 0;

    virtual bool is_supported(AFCodecID codec) = 0;

    static void addPrototype(audioRenderPrototype *se);

    static std::unique_ptr<Cicada::IAudioRender> create(AFCodecID codec);

};


#endif //CICADA_PLAYER_AUDIORENDERPROTOTYPE_H
