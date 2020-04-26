//
// Created by moqi on 2019/12/12.
//

#ifndef CICADAMEDIA_CICADAEVENTLISTENER_H
#define CICADAMEDIA_CICADAEVENTLISTENER_H


#include <MediaPlayer.h>
#include <memory>
#ifdef ENABLE_SDL
#include "SDLEventReceiver.h"
#include <SDL2/SDL.h>
#endif
#include "IEventReceiver.h"
#include <utils/timer.h>


class cicadaEventListener : public IEventReceiver::Listener {
public:
    explicit cicadaEventListener(Cicada::MediaPlayer *player) : mediaPlayer(player)
    {
        paused = false;
    }

    ~cicadaEventListener() override = default;

    void onExit() override;

    void onPausePlay() override;

    void onStepSeek(bool forward) override;

    void onPercentageSeek(int percent) override;

    void onChangeVolume(bool large) override;

    void onSetView(void *view) override;

    void onSpeedUp(bool up) override;

    void onPrePare() override;

    void onFullScreen(bool full) override;

protected:
    Cicada::MediaPlayer *mediaPlayer;
    bool paused = false;
};


#endif//CICADAMEDIA_CICADAEVENTLISTENER_H
