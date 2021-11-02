//
// Created by moqi on 2020/1/10.
//

#ifndef CICADAMEDIA_MEDIAPLAYERTEST_H
#define CICADAMEDIA_MEDIAPLAYERTEST_H

#include <string>

#include <MediaPlayer.h>

typedef int(*OnCallback)(Cicada::MediaPlayer *player, void* arg);

void test_simple(const string &url, OnCallback create, OnCallback loop, void *arg, playerListener *pListener,bool autoStart = true);
#ifdef ENABLE_SDL
#include <SDL2/SDL.h>
SDL_Window *getView();
void releaseView(SDL_Window *window);
#endif

#endif //CICADAMEDIA_MEDIAPLAYERTEST_H
