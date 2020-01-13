//
// Created by pingkai on 2020/1/13.
//

#include "player_command.h"

#ifdef ENABLE_SDL

#include <SDL2/SDL.h>
#include <utils/timer.h>

#endif

int command_loop(Cicada::MediaPlayer *player, void *arg)
{
    auto *pCommands = static_cast<vector<player_command> *>(arg);
    if (pCommands->empty()) {
        return -1;
    }
    player_command &cmd = pCommands->front();
#ifdef ENABLE_SDL
    SDL_Event event;
    if (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            return -1;
        }
    }
#endif
    if (cmd.timestamp < af_getsteady_ms()) {
        switch (cmd.mID) {
            case player_command::player_command_loop:
                player->SetLoop(cmd.arg0);
                break;
            case player_command::player_command_seek:
                player->SeekTo(cmd.arg0, SEEK_MODE_INACCURATE);
                break;
            case player_command::player_command_speed:
                player->SetSpeed((float) cmd.arg0 / 10);
                break;
            case player_command::player_command_volume:
                player->SetVolume((float) cmd.arg0 / 10);
                break;
            default:
                break;
        }
        pCommands->erase(pCommands->begin());
    }
    return 0;
}
