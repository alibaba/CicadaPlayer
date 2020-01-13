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

SDL_Window *window = nullptr;
SDL_Renderer *mVideoRender = nullptr;

static void *getView()
{
    if (window == nullptr) {
        SDL_Init(SDL_INIT_VIDEO);
        Uint32 flags = 0;
        flags |= SDL_WINDOW_ALLOW_HIGHDPI;
        flags |= SDL_WINDOW_RESIZABLE;
        SDL_CreateWindowAndRenderer(1280, 720, flags, &window, &mVideoRender);
    }
    return window;
}

static void releaseView()
{
    if (mVideoRender != nullptr) {
        SDL_DestroyRenderer(mVideoRender);
        mVideoRender = nullptr;
    }

    if (window != nullptr) {
        SDL_DestroyWindow(window);
        window = nullptr;
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

void test_player(const string &url, OnLoop loop, void *arg)
{
    unique_ptr<MediaPlayer> player = unique_ptr<MediaPlayer>(new MediaPlayer());
    player->SetDataSource(url.c_str());
    player->SetAutoPlay(true);
    player->SetView(getView());
    player->Prepare();
    if (loop) {
        int ret = 0;
        while (ret >= 0) {
            ret = loop(player.get(),arg);
        }
    } else
        af_msleep(10000);
    releaseView();
}
