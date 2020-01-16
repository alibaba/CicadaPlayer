//
// Created by pingkai on 2020/1/13.
//

#ifndef CICADAMEDIA_PLAYER_COMMAND_H
#define CICADAMEDIA_PLAYER_COMMAND_H


#include <MediaPlayer.h>
#include <vector>

class player_command {
public:
    enum command {
        null,
        seek,
        setLoop,
        start,
        setSpeed,
        setVolume,
        selectStream,
    };

    player_command() = default;

    player_command(command cmd, int ts) : mID(cmd), timestamp(ts)
    {

    }

    command mID{null};
    int timestamp{0};
    int arg0{};

};

class commandsCase {
public:
    commandsCase(std::vector<player_command> &cmds, bool exitOnEmpty) :
            mCommands(cmds), mExitOnEmpty(exitOnEmpty)
    {

    }

    std::vector<player_command> &mCommands;
    bool mExitOnEmpty{true};
};

int command_loop(Cicada::MediaPlayer *player, void *arg);

int simple_loop(Cicada::MediaPlayer *player, void *arg);


#endif //CICADAMEDIA_PLAYER_COMMAND_H
