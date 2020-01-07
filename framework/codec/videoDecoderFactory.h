//
// Created by moqi on 2018/11/8.
//

#ifndef CICADA_PLAYER_VIDEODECODERFACTORY_H
#define CICADA_PLAYER_VIDEODECODERFACTORY_H

#include <deque>
#include "IDecoder.h"


namespace Cicada{
    typedef struct IVideoDecoderFactory_t {
        bool (*isSupport)(enum AFCodecID codec);

        IDecoder *(*create)(enum AFCodecID codec, int flags);
    } IVideoDecoderFactory;

    class VideoDecoderFactoryManager {
    public:
        static void registerFactory(IVideoDecoderFactory *factory);

        static IDecoder *create(enum AFCodecID codec, int flag, bool buildIn);
    };
}


#endif //CICADA_PLAYER_VIDEODECODERFACTORY_H
