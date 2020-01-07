//
// Created by moqi on 2019-08-20.
//

#ifndef CICADA_PLAYER_DECODERFACTORY_H
#define CICADA_PLAYER_DECODERFACTORY_H

#include <memory>
#include "IDecoder.h"

class decoderFactory {

public:
    static std::unique_ptr<Cicada::IDecoder> create(AFCodecID codec, uint64_t flags, int maxSize);

private:

    static std::unique_ptr<Cicada::IDecoder> createBuildIn(const AFCodecID &codec, uint64_t flags);
};


#endif //CICADA_PLAYER_DECODERFACTORY_H
