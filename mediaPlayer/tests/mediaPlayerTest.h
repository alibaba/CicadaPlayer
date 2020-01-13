//
// Created by moqi on 2020/1/10.
//

#ifndef CICADAMEDIA_MEDIAPLAYERTEST_H
#define CICADAMEDIA_MEDIAPLAYERTEST_H

#include <string>

#include <MediaPlayer.h>

typedef int(*OnLoop)(Cicada::MediaPlayer *player,void* arg);

void test_player(const string &url, OnLoop loop, void *arg = nullptr);

#endif //CICADAMEDIA_MEDIAPLAYERTEST_H
