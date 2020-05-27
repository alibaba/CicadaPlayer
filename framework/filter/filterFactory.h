//
// Created by moqi on 2019-08-23.
//

#ifndef CICADA_PLAYER_FILTERFACTORY_H
#define CICADA_PLAYER_FILTERFACTORY_H

#include "IAudioFilter.h"

namespace Cicada {
    class filterFactory {

    public:
        static IAudioFilter *createAudioFilter(const IAudioFilter::format &srcFormat, const IAudioFilter::format &dstFormat, bool active);
    };
}// namespace Cicada

#endif //CICADA_PLAYER_FILTERFACTORY_H
