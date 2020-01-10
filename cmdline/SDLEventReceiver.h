//
// Created by moqi on 2019/12/11.
//

#ifndef CICADAMEDIA_SDLEVENTRECEIVER_H
#define CICADAMEDIA_SDLEVENTRECEIVER_H
#ifdef ENABLE_SDL
#include <SDL2/SDL.h>
#endif
#include "IEventReceiver.h"

class SDLEventReceiver : public IEventReceiver {

public:
    explicit SDLEventReceiver(Listener &listener);

    ~SDLEventReceiver();

    void poll(bool &exit) override;
private:
#ifdef ENABLE_SDL
    SDL_Event event{};
    SDL_Window *window = nullptr;
    SDL_Renderer *mVideoRender = nullptr;
#endif
};


#endif //CICADAMEDIA_SDLEVENTRECEIVER_H
