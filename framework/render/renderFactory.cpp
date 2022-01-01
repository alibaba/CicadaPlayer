//
// Created by moqi on 2019-08-20.
//
#include "renderFactory.h"

#ifdef ANDROID
    #include "audio/Android/AudioTrackRender.h"
#endif

#ifdef __APPLE__
#include <render/audio/Apple/AFAudioQueueRender.h>
#include <render/video/AVFoundation/AVFoundationVideoRender.h>
#endif


#if TARGET_OS_OSX

#endif
#ifdef ENABLE_SDL

#include "audio/SdlAFAudioRender2.h"
#include "video/SdlAFVideoRender.h"

#endif

#ifdef GLRENDER

#include "video/glRender/GLRender.h"

#endif

#include "audio/audioRenderPrototype.h"

#ifdef ENABLE_CHEAT_RENDER

#include "audio/CheaterAudioRender.h"

#endif

#include "video/DummyVideoRender.h"

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
#elif defined(ENABLE_SDL)
    return std::unique_ptr<IAudioRender>(new SdlAFAudioRender2());
#endif
    return nullptr;
}

unique_ptr<IVideoRender> videoRenderFactory::create(uint64_t flags)
{
    if (flags & IVideoRender::FLAG_DUMMY) {
        return std::unique_ptr<IVideoRender>(new DummyVideoRender());
    }

    if (flags & IVideoRender::FLAG_HDR) {
#if defined(__APPLE__) && !defined(ENABLE_SDL)
        return std::unique_ptr<IVideoRender>(new AVFoundationVideoRender());
#endif
        return nullptr;
    }

#if defined(GLRENDER)
    return std::unique_ptr<IVideoRender>(new GLRender());
#elif defined(ENABLE_SDL)
    return std::unique_ptr<IVideoRender>(new SdlAFVideoRender());
#elif defined(ENABLE_CHEAT_RENDER)
    return std::unique_ptr<IVideoRender>(new DummyVideoRender());
#endif
    return nullptr;
}
