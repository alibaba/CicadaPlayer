//
// Created by moqi on 2020/4/24.
//

#ifndef CICADAMEDIA_SYNCPLAYEREVENTLISTENER_H
#define CICADAMEDIA_SYNCPLAYEREVENTLISTENER_H
#include "../cicadaEventListener.h"
#include "communication/playerMessage.h"
#include <communication/messageServer.h>
namespace Cicada {
    class syncPlayerEventListener : public cicadaEventListener {
    public:
        syncPlayerEventListener(MediaPlayer *player, messageServer *server, void *window)
            : cicadaEventListener(player), mServer(server), mWindow(window)
        {}
        ~syncPlayerEventListener() override = default;

        void onPrePare() override
        {
            mediaPlayer->Prepare();
            if (mServer) {
                mServer->write(playerMessage::prepare);
            }
        }

        void onFullScreen(bool full) override
        {
#ifdef ENABLE_SDL
            if (mWindow) {
                SDL_SetWindowFullscreen((SDL_Window *) mWindow, full ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
            }
#endif
            if (mServer) {
                mServer->write(playerMessage::fullScreen);
                mServer->write(full ? "1" : "0");
            }
        }

    private:
        messageServer *mServer;
        void *mWindow;
    };
}// namespace Cicada


#endif//CICADAMEDIA_SYNCPLAYEREVENTLISTENER_H
