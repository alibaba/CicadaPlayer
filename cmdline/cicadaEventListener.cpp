//
// Created by moqi on 2019/12/12.
//

#define MAX_SPEED 5
#define MIN_SPEED 0.5
#include <utils/globalNetWorkManager.h>

#include "cicadaEventListener.h"
using namespace Cicada;

void cicadaEventListener::onExit()
{
    mediaPlayer->Stop();
}

void cicadaEventListener::onPausePlay()
{
    if (paused) {
        mediaPlayer->Start();
    } else {
        mediaPlayer->Pause();
    }

    paused = !paused;
}

void cicadaEventListener::onStepSeek(bool forward)
{
    int time = 10000;

    if (!forward) {
        time = -time;
    }

    int64_t pos = mediaPlayer->GetCurrentPosition() + time;

    mediaPlayer->SeekTo(pos < 0 ? 0 : pos, SEEK_MODE_INACCURATE);
}
void cicadaEventListener::onPercentageSeek(int percent)
{
    mediaPlayer->SeekTo(mediaPlayer->GetDuration() / 100 * percent, SEEK_MODE_INACCURATE);
}

void cicadaEventListener::onChangeVolume(bool large)
{
    float vol = mediaPlayer->GetVolume();

    if (large) {
        vol += 0.1;
    } else {
        vol -= 0.1;
    }

    mediaPlayer->SetVolume(vol);
}

void cicadaEventListener::onSetView(void *view)
{
    mediaPlayer->SetView(view);
}

void cicadaEventListener::onSpeedUp(bool up)
{
    float speed = mediaPlayer->GetSpeed();

    if (up) {
        speed += 0.1;
    } else {
        speed -= 0.1;
    }

    if (speed <= MAX_SPEED && speed >= MIN_SPEED) {
        mediaPlayer->SetSpeed(speed);
    }
}
void cicadaEventListener::onPrePare()
{
    //   mediaPlayer->Prepare();
}
void cicadaEventListener::onFullScreen(bool full)
{}
void cicadaEventListener::onSpeedReset()
{
    mediaPlayer->SetSpeed(1.0f);
}
void cicadaEventListener::onReconnect()
{
    globalNetWorkManager::getGlobalNetWorkManager()->reConnect();
}
