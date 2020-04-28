#include <MediaPlayer.h>
#include <memory>
#include <utils/timer.h>

using namespace Cicada;
using namespace std;

#include "NetWorkEventReceiver.h"
#include "SDLEventReceiver.h"
#include "cicadaEventListener.h"
#include <communication/messageServer.h>

#ifdef ENABLE_SDL
#include <SDL2/SDL_main.h>
#define SDL_MAIN_HANDLED
#endif

#include "communication/playerMessage.h"
#include "syncPlayerEventListener.h"
#include <media_player_error_def.h>
#include <utils/af_clock.h>

using IEvent = IEventReceiver::IEvent;
struct cicadaCont {
    unique_ptr<MediaPlayer> player{};
    IEventReceiver *receiver{};
    bool error{};
    bool bMaster{};
    messageServer *server{};
    af_clock masterClock;
    PlayerStatus status{};
    bool loading{};
#ifdef ENABLE_SDL
    SDL_Window *window{};
    SDL_Renderer *rendere{};
#endif
};

static void onStatusChanged(int64_t oldStatus, int64_t newStatus, void *userData)
{
    AF_TRACE;
    auto *cont = static_cast<cicadaCont *>(userData);

    if (cont->server) {
        if (newStatus == PLAYER_PLAYING)
            cont->server->write(playerMessage::start);
        else if (newStatus == PLAYER_PAUSED) {
            cont->server->write(playerMessage::pause);
        }
    }
    cont->status = static_cast<PlayerStatus>(newStatus);
}
static void onLoadingStart(void *userData)
{
    auto *cont = static_cast<cicadaCont *>(userData);
    if (cont->server) {
        cont->server->write(playerMessage::pause);
    }
    cont->loading = true;
}
static void onLoadingEnd(void *userData)
{
    auto *cont = static_cast<cicadaCont *>(userData);
    if (cont->server) {
        cont->server->write(playerMessage::start);
    }
    cont->loading = false;
}

static void onPositionUpdate(int64_t pos, void *userData)
{
    auto *cont = static_cast<cicadaCont *>(userData);
    if (cont->server) {
        cont->server->write(playerMessage::clock);
        cont->server->write(to_string(cont->player->GetMasterClockPts()));
    }
}

static void onEOS(void *userData)
{
    auto *cont = static_cast<cicadaCont *>(userData);

    if (cont->receiver) {
        cont->receiver->push(std::unique_ptr<IEvent>(new IEvent(IEvent::TYPE_EXIT)));
    }
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

    if (cont->receiver) {
        auto *event = new IEvent(IEvent::TYPE_EXIT);
        cont->receiver->push(std::unique_ptr<IEvent>(event));
    } else {
        cont->error = true;
    }
}

static MediaPlayer *createPlayer(cicadaCont &cicada, const string &url)
{
    cicada.player = unique_ptr<MediaPlayer>(new MediaPlayer());
    playerListener pListener{nullptr};
    pListener.userData = &cicada;
    pListener.Completion = onEOS;
    pListener.EventCallback = onEvent;
    pListener.ErrorCallback = onError;
    pListener.StatusChanged = onStatusChanged;
    pListener.LoadingStart = onLoadingStart;
    pListener.LoadingEnd = onLoadingEnd;
    pListener.PositionUpdate = onPositionUpdate;
    cicada.player->SetListener(pListener);
    cicada.player->SetDefaultBandWidth(1000 * 1000);
    cicada.player->SetDataSource(url.c_str());
#ifdef ENABLE_SDL
    Uint32 flags = 0;
    flags |= SDL_WINDOW_ALLOW_HIGHDPI;
    flags |= SDL_WINDOW_RESIZABLE;
    SDL_CreateWindowAndRenderer(640, 360, flags, &(cicada.window), &(cicada.rendere));
    SDL_SetWindowTitle(cicada.window, cicada.bMaster ? "master" : "slave");
    cicada.player->SetView(cicada.window);
#else
    int view = 0;
    cicada.player->SetView(&view);
#endif
    return cicada.player.get();
}
static int64_t getMasterClock(void *arg)
{
    auto *cont = static_cast<cicadaCont *>(arg);
    return cont->masterClock.get();
};

static string serverIp = "tcp://localhost:8888";

class syncServerListener : public IProtocolServer::Listener {
public:
    explicit syncServerListener(cicadaCont *cicada) : mCicada(cicada)
    {}

    void onAccept(IProtocolServer::IClient **client) override
    {

        if (mCicada->status >= PLAYER_PREPARINIT) {
            Cicada::messageServer::write(playerMessage::seekAccurate, *client);
            Cicada::messageServer::write(to_string(mCicada->player->GetCurrentPosition()), *client);
            Cicada::messageServer::write(playerMessage::prepare, *client);
        }

        if (mCicada->status == PLAYER_PLAYING && !mCicada->loading) {
            Cicada::messageServer::write(playerMessage::start, *client);
        }
    }

private:
    cicadaCont *mCicada{};
};

static void process_client_msg(cicadaCont &cicada, unique_ptr<messageClient> &client, string &msg, bool &quite)
{
    msg = client->readMessage();
    if (!msg.empty()) {
        AF_LOGE("client msg %s\n", msg.c_str());
        if (msg == playerMessage::start) {
            cicada.player->Start();
            cicada.masterClock.start();
        } else if (msg == playerMessage::pause) {
            cicada.player->Pause();
            cicada.masterClock.pause();
        } else if (msg == playerMessage::exit) {
            quite = true;
        } else if (msg == playerMessage::prepare) {
            cicada.player->Prepare();
        } else if (msg == playerMessage::clock) {
            int64_t pts = atoll(client->readMessage().c_str());
            cicada.masterClock.set(pts);
            if (llabs(pts - cicada.player->GetMasterClockPts()) > 40000) {
                AF_LOGW("delta pts is %lld\n", pts - cicada.player->GetMasterClockPts());
            }
        } else if (msg == playerMessage::fullScreen) {
#ifdef ENABLE_SDL
            bool full = atoll(client->readMessage().c_str()) != 0;
            SDL_SetWindowFullscreen(cicada.window, full ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
#endif
        } else if (msg == playerMessage::seekAccurate) {
            int64_t ms = atoll(client->readMessage().c_str());
            cicada.player->SeekTo(ms, SEEK_MODE_ACCURATE);
        } else if (msg == playerMessage::seek) {
            int64_t ms = atoll(client->readMessage().c_str());
            cicada.player->SeekTo(ms, SEEK_MODE_INACCURATE);
        }
    }
}
int main(int argc, const char **argv)
{
    string url;
    bool connected = false;
    bool bMaster = true;
    if (argc > 2) {
        bMaster = false;
    }
    if (bMaster) {
        AF_LOGW("master");
    }

    if (argc > 1) {
        url = argv[1];
    } else {
        url = "http://player.alicdn.com/video/aliyunmedia.mp4";
    }

    log_enable_color(1);
    log_set_level(AF_LOG_LEVEL_WARNING, 1);
    cicadaCont cicada{};
#ifdef ENABLE_SDL
    SDL_Init(SDL_INIT_VIDEO);
#endif
    cicada.bMaster = bMaster;
    createPlayer(cicada, url);
    unique_ptr<messageClient> client{};
    unique_ptr<messageServer> server{};

    syncServerListener syncListener(&cicada);

    if (bMaster) {
        cicada.player->SetAutoPlay(true);
        //    player->SetLoop(true);
        server = static_cast<unique_ptr<messageServer>>(new messageServer(&syncListener));
        server->init();
        cicada.server = server.get();
    } else {
        cicada.player->SetStreamTypeFlags(VIDEO_FLAG);
        cicada.player->SetClockRefer(getMasterClock, &cicada);
        client = static_cast<unique_ptr<messageClient>>(new messageClient());
        int ret = client->connect(serverIp);
        if (ret >= 0) {
            connected = true;
        }
    }
#ifdef ENABLE_SDL
    syncPlayerEventListener eListener(cicada.player.get(), server.get(), cicada.window);
    SDLEventReceiver receiver(eListener);
#else
    syncPlayerEventListener eListener(cicada.player.get(), server.get(), nullptr);
#endif
    NetWorkEventReceiver netWorkEventReceiver(eListener);

    cicada.player->SelectTrack(-1);


    bool quite = false;
    string msg{};

    while (!quite && !cicada.error) {

#ifdef ENABLE_SDL
        receiver.poll(quite);
#endif

        if (!bMaster) {
            if (!connected) {
                connected = client->connect(serverIp) >= 0;
            } else {
                process_client_msg(cicada, client, msg, quite);
            }
        } else {
            if (!quite) {
                netWorkEventReceiver.poll(quite);
            }
        }
        if (quite) {
            auto *event = new IEvent(IEvent::TYPE_EXIT);
#ifdef ENABLE_SDL
            receiver.push(std::unique_ptr<IEvent>(event));
#endif
            if (server) {
                server->write(playerMessage::exit);
            }
        } else {
            af_msleep(10);
        }
    }

    return 0;
}