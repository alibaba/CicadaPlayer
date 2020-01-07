//
// Created by moqi on 2019/12/11.
//

#ifndef CICADAMEDIA_SDLEVENTRECEIVER_H
#define CICADAMEDIA_SDLEVENTRECEIVER_H
#include <SDL2/SDL.h>
#include "IEventReceiver.h"

class SDLEventReceiver : public IEventReceiver {

public:
    explicit SDLEventReceiver(Listener &listener);

    ~SDLEventReceiver();

    void poll(bool &exit) override;
private:
    SDL_Event event{};

private:
    SDL_Window *window = nullptr;
    SDL_Renderer *mVideoRender = nullptr;
};


#endif //CICADAMEDIA_SDLEVENTRECEIVER_H
