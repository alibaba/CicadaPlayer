#include <MediaPlayer.h>
#include <memory>
#include <utils/timer.h>

using namespace Cicada;
using namespace std;
#define SDL_MAIN_HANDLED
#include "SDLEventReceiver.h"
#include "cicadaEventListener.h"
#include "NetWorkEventReceiver.h"
#include <SDL2/SDL_main.h>
#include <media_player_error_def.h>
using IEvent = IEventReceiver::IEvent;
struct cicadaCont {
    MediaPlayer *player;
    IEventReceiver *receiver;
};

static void onVideoSize(int64_t width, int64_t height, void *userData)
{
    AF_TRACE;
    using IEvent = IEventReceiver::IEvent;
    auto *cont = static_cast<cicadaCont *>(userData);
    auto *event = new IEvent(IEvent::TYPE_SET_VIEW);
    cont->receiver->push(std::unique_ptr<IEvent>(event));
}

static void onEOS(void *userData)
{
    auto *cont = static_cast<cicadaCont *>(userData);
    auto *event = new IEvent(IEvent::TYPE_EXIT);
    cont->receiver->push(std::unique_ptr<IEvent>(event));
}

static void onEvent(int64_t errorCode, const void *errorMsg, void *userData)
{
    auto *cont = static_cast<cicadaCont *>(userData);

    if (errorMsg) {
        AF_LOGD("%s\n", errorMsg);
    }

    switch (errorCode) {
        case MediaPlayerEventType::MEDIA_PLAYER_EVENT_NETWORK_RETRY:
            cont->player->Reload();
            break;

        default:
            break;
    }
}

static void onError(int64_t errorCode, const void *errorMsg, void *userData)
{
    auto *cont = static_cast<cicadaCont *>(userData);

    if (errorMsg) {
        AF_LOGE("%s\n", errorMsg);
    }

    auto *event = new IEvent(IEvent::TYPE_EXIT);
    cont->receiver->push(std::unique_ptr<IEvent>(event));
}

int main(int argc, const char **argv)
{
    string url;

    if (argc > 1) {
        url = argv[1];
    } else {
        url = "http://player.alicdn.com/video/aliyunmedia.mp4";
    }

    cicadaCont cicada{};
    unique_ptr<MediaPlayer> player = unique_ptr<MediaPlayer>(new MediaPlayer());
    cicada.player = player.get();
    playerListener pListener{nullptr};
    pListener.userData = &cicada;
    pListener.VideoSizeChanged = onVideoSize;
    pListener.Completion = onEOS;
    pListener.EventCallback = onEvent;
    pListener.ErrorCallback = onError;
    cicadaEventListener eListener(player.get());
    SDLEventReceiver receiver(eListener);
    NetWorkEventReceiver netWorkEventReceiver(eListener);
    cicada.receiver = &receiver;
    player->SetListener(pListener);
    player->SetDefaultBandWidth(100000000);
    player->SetDataSource(url.c_str());
    player->SetAutoPlay(true);
    player->SetLoop(true);
    player->Prepare();
    bool quite = false;

    while (!quite) {
        receiver.poll(quite);

        if (!quite) {
            netWorkEventReceiver.poll(quite);

            if (quite) {
                auto *event = new IEvent(IEvent::TYPE_EXIT);
                receiver.push(std::unique_ptr<IEvent>(event));
            }
        }

        if (!quite) {
            af_msleep(10);
        }
    }

    return 0;
}