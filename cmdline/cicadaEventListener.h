//
// Created by moqi on 2019/12/12.
//

#ifndef CICADAMEDIA_CICADAEVENTLISTENER_H
#define CICADAMEDIA_CICADAEVENTLISTENER_H


#include <MediaPlayer.h>
#include <memory>
#ifdef ENABLE_SDL
#include <SDL2/SDL.h>
#include "SDLEventReceiver.h"
#endif
#include <utils/timer.h>
#include "IEventReceiver.h"


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

private:
    Cicada::MediaPlayer *mediaPlayer;
    bool paused = false;
};


#endif //CICADAMEDIA_CICADAEVENTLISTENER_H
