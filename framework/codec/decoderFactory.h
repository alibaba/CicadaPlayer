//
// Created by moqi on 2019-08-20.
//

#ifndef CICADA_PLAYER_DECODERFACTORY_H
#define CICADA_PLAYER_DECODERFACTORY_H

#include <memory>
#include <map>
#include "IDecoder.h"

class decoderFactory {

public:
    static std::unique_ptr<Cicada::IDecoder> create(const Stream_meta & meta, uint64_t flags, int maxSize ,
                                                    const Cicada::DrmInfo *drmInfo);

private:

    static std::unique_ptr<Cicada::IDecoder> createBuildIn(const AFCodecID &codec, uint64_t flags,
                                                           const Cicada::DrmInfo *drmInfo);
};


#endif //CICADA_PLAYER_DECODERFACTORY_H
