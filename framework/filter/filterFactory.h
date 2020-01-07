//
// Created by moqi on 2019-08-23.
//

#ifndef CICADA_PLAYER_FILTERFACTORY_H
#define CICADA_PLAYER_FILTERFACTORY_H

#include "IAudioFilter.h"

class filterFactory {

public:
    static Cicada::IAudioFilter *
    createAudioFilter(const Cicada::IAudioFilter::format &srcFormat, const Cicada::IAudioFilter::format &dstFormat);

};


#endif //CICADA_PLAYER_FILTERFACTORY_H
