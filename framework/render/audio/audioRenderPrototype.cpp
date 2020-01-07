//
// Created by moqi on 2019/10/21.
//

#include "audioRenderPrototype.h"

audioRenderPrototype *audioRenderPrototype::renderQueue[];
int audioRenderPrototype::_nextSlot;

void audioRenderPrototype::addPrototype(audioRenderPrototype *se)
{
    renderQueue[_nextSlot++] = se;
}

std::unique_ptr<Cicada::IAudioRender> audioRenderPrototype::create(AFCodecID codec)
{
    for (int i = 0; i < _nextSlot; ++i) {
        if (renderQueue[i]->is_supported(codec)) {
            return std::unique_ptr<Cicada::IAudioRender>(renderQueue[i]->clone());
        }
    }

    return nullptr;
}
