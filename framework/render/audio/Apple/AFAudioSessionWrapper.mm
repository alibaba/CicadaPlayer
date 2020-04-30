//
// Created by moqi on 2019/10/21.
//

#import <mutex>
#import <render/audio/IAudioRender.h>
#import <list>
#import "AFAudioSessionWrapper.h"
#import "AFAudioSession.h"

std::mutex gAFRenderMutex;

std::list<AFAudioSessionWrapper::listener *> AFAudioSessionWrapper::mListenerList;

void AFAudioSessionWrapper::init()
{
    std::function<void(AF_AUDIO_SESSION_STATUS)> fun = [](AF_AUDIO_SESSION_STATUS state) -> void {
        return onInterruption(state);
    };

    [[AFAudioSession sharedInstance] setCallback:fun];
}

void AFAudioSessionWrapper::addObserver(listener *li)
{
    mListenerList.push_back(li);
}

void AFAudioSessionWrapper::removeObserver(listener *li)
{
    mListenerList.remove(li);
}

void AFAudioSessionWrapper::onInterruption(AF_AUDIO_SESSION_STATUS status)
{
    for (auto &listener : mListenerList) {
        listener->onInterrupted(status);
    }
}

// call from main thread
int AFAudioSessionWrapper::activeAudio()
{
    return [[AFAudioSession sharedInstance] activeAudio];
}
