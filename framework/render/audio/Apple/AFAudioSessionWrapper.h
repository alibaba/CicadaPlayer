//
// Created by moqi on 2019/10/21.
//

#ifndef CICADA_PLAYER_AFAUDIOSESSIONWRAPPER_H
#define CICADA_PLAYER_AFAUDIOSESSIONWRAPPER_H

#include "AudioRenderType.h"
#include <list>

class AFAudioSessionWrapper {

public:
    class listener {
    public:
        virtual void onInterrupted(Cicada::AF_AUDIO_SESSION_STATUS status) = 0;

        virtual ~listener() = default;
    };

public:
    static void init();

    static void addObserver(listener *li);

    static void removeObserver(listener *li);

    static int activeAudio();

private:
    static void onInterruption(Cicada::AF_AUDIO_SESSION_STATUS status);

private:
    static std::list<listener *> mListenerList;
};

#endif //CICADA_PLAYER_AFAUDIOSESSIONWRAPPER_H
