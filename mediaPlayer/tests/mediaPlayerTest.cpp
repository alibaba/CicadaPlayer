//
// Created by moqi on 2020/1/10.
//

#include <utils/timer.h>
#include "mediaPlayerTest.h"
using namespace std;
using namespace Cicada;


#ifdef ENABLE_SDL

#include <SDL2/SDL.h>
#include <utils/timer.h>

static SDL_Window *getView()
{
    SDL_Window *window = nullptr;
    SDL_Renderer *mVideoRender = nullptr;
    SDL_Init(SDL_INIT_VIDEO);
    Uint32 flags = 0;
    flags |= SDL_WINDOW_ALLOW_HIGHDPI;
    flags |= SDL_WINDOW_RESIZABLE;
    SDL_CreateWindowAndRenderer(1280, 720, flags, &window, &mVideoRender);
    return window;
}

static void releaseView(SDL_Window *window)
{
    if (window != nullptr) {
        SDL_Renderer *render = SDL_GetRenderer(window);
        if(render)
            SDL_DestroyRenderer(render);
        SDL_DestroyWindow(window);
    }
}

#else
static void *getView(){
    int i;
    return &i;
}
static void releaseView(){

}

#endif

void test_simple(const string &url, OnCallback create, OnCallback loop, void *arg, playerListener *pListener, bool autoStart)
{
    unique_ptr<MediaPlayer> player = unique_ptr<MediaPlayer>(new MediaPlayer());
    if (create) {
        create(player.get(), arg);
    }
#ifdef ENABLE_SDL
    SDL_Window *window = getView();
    player->SetView(window);
#else
    int i;
    player->SetView(&i);
#endif
    if (pListener) {
        player->SetListener(*pListener);
    }
    player->SetDataSource(url.c_str());
    player->SetAutoPlay(autoStart);
    player->Prepare();
    if (loop) {
        int ret = 0;
        while (ret >= 0) {
            ret = loop(player.get(),arg);
        }
    } else
        af_msleep(10000);
#ifdef ENABLE_SDL
    releaseView(window);
#endif
}

