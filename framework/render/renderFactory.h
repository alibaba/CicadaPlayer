//
// Created by moqi on 2019-08-20.
//

#ifndef CICADA_PLAYER_RENDERFACTORY_H
#define CICADA_PLAYER_RENDERFACTORY_H

#include "render/audio/IAudioRender.h"
#include "render/video/IVideoRender.h"
namespace Cicada {
    class AudioRenderFactory {
    public:
        static std::unique_ptr<IAudioRender> create();
    };

    class videoRenderFactory {
    public:
        static const uint64_t FLAG_HDR = (1 << 0);
        static std::unique_ptr<IVideoRender> create(uint64_t flags = 0);
    };
}// namespace Cicada

#endif//CICADA_PLAYER_RENDERFACTORY_H
