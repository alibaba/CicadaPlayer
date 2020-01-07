//
// Created by moqi on 2019-08-20.
//
#include "renderFactory.h"

#ifdef ANDROID
    #include "audio/Android/AudioTrackRender.h"
#endif

#ifdef __APPLE__
    #include <render/audio/Apple/AFAudioUnitRender.h>
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

#include "audio/audioRenderPrototype.h"
using namespace Cicada;

std::unique_ptr<IAudioRender> AudioRenderFactory::create()
{
    std::unique_ptr<IAudioRender> render = audioRenderPrototype::create(AF_CODEC_ID_NONE);

    if (render) {
        return render;
    }

#ifdef ANDROID
    return std::unique_ptr<IAudioRender>(new AudioTrackRender());
#endif
#ifdef __APPLE__
    return std::unique_ptr<IAudioRender>(new AFAudioUnitRender());
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
#endif
    return nullptr;
}
