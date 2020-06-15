//
// Created by moqi on 2019/12/11.
//

#include "SDLEventReceiver.h"
#if USE_NATIVE_WINDOW
extern "C" {
#include "nativeWindow/nativeWindow.h"
}
#endif
#include <SDL2/SDL_syswm.h>
#include <utils/frame_work_log.h>
#include <utils/timer.h>

SDLEventReceiver::SDLEventReceiver(IEventReceiver::Listener &listener) : IEventReceiver(listener) {
}

SDLEventReceiver::~SDLEventReceiver() = default;

void SDLEventReceiver::poll(bool &exit) {
    std::unique_ptr<IEvent> UserEvent = pop();

    if (UserEvent) {
        switch (UserEvent->getType()) {
        case IEvent::TYPE_SET_VIEW: {
            if (SDL_WasInit(SDL_INIT_VIDEO) != SDL_INIT_VIDEO) {
                SDL_Init(SDL_INIT_VIDEO);
            }
            if (mView.view == nullptr) {
#if USE_NATIVE_WINDOW
                if (getNativeFactor() != nullptr) {
                    mView.view = (getNativeFactor()->CreateNativeWindow(1280, 720));
                    if (mView.view) {
                        mView.type = CicadaSDLViewType_NATIVE_WINDOW;
                    }
                }

#endif
                if (mView.view == nullptr) {
                    Uint32 flags = 0;
                    flags |= SDL_WINDOW_ALLOW_HIGHDPI;
                    flags |= SDL_WINDOW_RESIZABLE;
                    SDL_Renderer *renderer;
                    SDL_CreateWindowAndRenderer(1280, 720, flags, reinterpret_cast<SDL_Window **>(&mView.view), &renderer);
                    mView.type = CicadaSDLViewType_SDL_WINDOW;
                }
            }
            mListener.onSetView(&mView);
            break;
        }
        case IEvent::TYPE_EXIT:
          mListener.onExit();
          exit = true;
          return;

        default:
          AF_LOGW("unknown type event %d", UserEvent->getType());
          break;
        }
    }

    if (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                mListener.onExit();
                exit = true;
                break;

            case SDL_KEYDOWN: {
                switch (event.key.keysym.sym) {
                    case SDLK_SPACE:
                        mListener.onPausePlay();
                        break;

                    case SDLK_ESCAPE:
                        mListener.onExit();
                        exit = true;
                        break;

                    case SDLK_RIGHT:
                    case SDLK_LEFT:
                        mListener.onStepSeek(event.key.keysym.sym == SDLK_RIGHT);
                        break;

                    case SDLK_UP:
                    case SDLK_DOWN:
                        mListener.onChangeVolume(event.key.keysym.sym == SDLK_UP);
                        break;

                    case SDLK_F7:
                    case SDLK_F9:
                        mListener.onSpeedUp(event.key.keysym.sym == SDLK_F9);
                        break;

                    case SDLK_p:
                        mListener.onPrePare();
                        break;

                    default:
                        if (event.key.keysym.sym >= SDLK_0 && event.key.keysym.sym <= SDLK_9) {
                            mListener.onPercentageSeek((event.key.keysym.sym - SDLK_0) * 10);
                        } else
                            AF_LOGW("unknown key %c", event.key.keysym.sym);
                        break;
                }
                break;
            }

            case SDL_MOUSEBUTTONDOWN: {
                if (event.button.button == SDL_BUTTON_LEFT && mView.type == CicadaSDLViewType_SDL_WINDOW) {
                    static int64_t last_mouse_left_click = 0;
                    static bool is_full_screen = false;
                    if (af_gettime_relative() - last_mouse_left_click <= 500000) {
                        is_full_screen = !is_full_screen;
                        if (mView.view) {
                            SDL_SetWindowFullscreen((SDL_Window *) mView.view, is_full_screen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
                        }
                        mListener.onFullScreen(is_full_screen);
                        last_mouse_left_click = 0;
                    } else {
                        last_mouse_left_click = af_gettime_relative();
                    }
                }
            }
                break;

            default:
                break;
        }
        if (exit){
            if (mView.view != nullptr) {
#if USE_NATIVE_WINDOW
                if (mView.type == CicadaSDLViewType_NATIVE_WINDOW) {
                    getNativeFactor()->DestroyNativeWindow(mView.view);
                } else
#endif
                    SDL_DestroyWindow((SDL_Window *) mView.view);
                mView.view = nullptr;
            }
        }
    }
}
