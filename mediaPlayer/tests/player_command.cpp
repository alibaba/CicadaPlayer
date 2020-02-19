//
// Created by pingkai on 2020/1/13.
//

#include "player_command.h"
#include <utils/timer.h>

#ifdef ENABLE_SDL

    #include <SDL2/SDL.h>

#endif

int command_loop(Cicada::MediaPlayer *player, void *arg)
{
#ifdef ENABLE_SDL
    SDL_Event event;

    if (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return -1;
        }
    }

#endif
    commandsCase *pCase = static_cast<commandsCase *> (arg);
    std::unique_lock <std::mutex>lock(pCase->mMutex);

    if (pCase->mCommands.empty()) {
        if (pCase->mExitOnEmpty) {
            return -1;
        }

        return 0;
    }

    player_command &cmd = pCase->mCommands.front();

    if (cmd.timestamp < af_getsteady_ms()) {
        switch (cmd.mID) {
            case player_command::setLoop:
                player->SetLoop(cmd.arg0);
                break;

            case player_command::seek:
                player->SeekTo(player->GetCurrentPosition() + cmd.arg0, SEEK_MODE_ACCURATE);
                break;

            case player_command::setSpeed:
                player->SetSpeed((float) cmd.arg0 / 10);
                break;

            case player_command::setVolume:
                player->SetVolume((float) cmd.arg0 / 10);
                break;

            case player_command::selectStream:
                player->SelectTrack(cmd.arg0);
                break;

            case player_command::start:
                player->Start();
                break;

            case player_command::backGround:
                player->EnterBackGround(cmd.arg0 != 0);
                break;

            default:
                break;
        }

        pCase->mCommands.erase( pCase->mCommands.begin());
    }

    return 0;
}

static int g_play_time_s = 10;

int simple_loop(Cicada::MediaPlayer *player, void *arg)
{
#ifdef ENABLE_SDL
    int64_t start = af_getsteady_ms();
    SDL_Event event;

    while (true) {
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                break;
            }
        }

        af_msleep(10);

        if (af_getsteady_ms() - start > g_play_time_s * 1000) {
            break;
        }
    }

#else
    af_msleep(g_play_time_s * 1000);
#endif
    return -1;
}
