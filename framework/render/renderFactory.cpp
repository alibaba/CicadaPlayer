//
// Created by moqi on 2019-08-20.
//
#include "renderFactory.h"

#ifdef ANDROID
    #include "audio/Android/AudioTrackRender.h"
#endif

#ifdef __APPLE__
    #include <render/audio/Apple/AFAudioQueueRender.h>
#endif


#if TARGET_OS_OSX

#endif
#ifdef ENABLE_SDL

#include "audio/SdlAFAudioRender.h"
#include "video/SdlAFVideoRender.h"

#endif

#ifdef GLRENDER

#include "video/glRender/GLRender.h"

#endif

#ifdef ENABLE_CHEAT_RENDER

#include "video/CheaterVideoRender.h"

#endif

#include "audio/audioRenderPrototype.h"

#ifdef ENABLE_CHEAT_RENDER

#include "audio/CheaterAudioRender.h"

#endif

using namespace Cicada;

std::unique_ptr<IAudioRender> AudioRenderFactory::create()
{
    std::unique_ptr<IAudioRender> render = audioRenderPrototype::create(AF_CODEC_ID_NONE);

    if (render) {
        return render;
    }
#ifdef ENABLE_CHEAT_RENDER
    return std::unique_ptr<IAudioRender>(new CheaterAudioRender());
#endif

#ifdef ANDROID
    return std::unique_ptr<IAudioRender>(new AudioTrackRender());
#endif
#ifdef __APPLE__
    return std::unique_ptr<IAudioRender>(new AFAudioQueueRender());
#endif
#ifdef ENABLE_SDL
    return std::unique_ptr<IAudioRender>(new SdlAFAudioRender());
#endif
    return nullptr;
}

std::unique_ptr<IVideoRender> videoRenderFactory::create()
{
#if defined(GLRENDER)
    return std::unique_ptr<IVideoRender>(new GLRender());
#elif defined(ENABLE_SDL)
    return std::unique_ptr<IVideoRender>(new SdlAFVideoRender());
#elif defined(ENABLE_CHEAT_RENDER)
    return std::unique_ptr<IVideoRender>(new CheaterVideoRender());
#endif
    return nullptr;
}
