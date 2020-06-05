//
// Created by moqi on 2019/12/11.
//

#ifndef CICADAMEDIA_SDLEVENTRECEIVER_H
#define CICADAMEDIA_SDLEVENTRECEIVER_H
#ifdef ENABLE_SDL
#include <SDL2/SDL.h>
#endif
#include "IEventReceiver.h"
#define USE_NATIVE_WINDOW 0

enum CicadaSDLViewType { CicadaSDLViewType_SDL_WINDOW, CicadaSDLViewType_NATIVE_WINDOW };

typedef struct CicadaSDLView_t {
    void *view;
    CicadaSDLViewType type;
} CicadaSDLView;

class SDLEventReceiver : public IEventReceiver {

public:
    explicit SDLEventReceiver(Listener &listener);

    ~SDLEventReceiver();

    void poll(bool &exit) override;
private:
#ifdef ENABLE_SDL
    SDL_Event event{};
    CicadaSDLView mView{};
#endif
};


#endif //CICADAMEDIA_SDLEVENTRECEIVER_H
