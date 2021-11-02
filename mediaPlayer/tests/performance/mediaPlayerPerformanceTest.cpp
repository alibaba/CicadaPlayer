//
// Created by moqi on 2021/11/1.
//
#include "tests/mediaPlayerTest.h"
#include "tests/player_command.h"
#include "gtest/gtest.h"
#include <utils/frame_work_log.h>
#include <utils/globalSettings.h>
#include <utils/timer.h>
using namespace Cicada;

enum CicadaSDLViewType { CicadaSDLViewType_SDL_WINDOW, CicadaSDLViewType_NATIVE_WINDOW };
typedef struct CicadaSDLView {
    void *view;
    CicadaSDLViewType type;
} CicadaSDLView;

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    log_set_level(AF_LOG_LEVEL_TRACE, 1);
    log_enable_color(1);
    //    ignore_signal(SIGPIPE);
    return RUN_ALL_TESTS();
}

typedef struct prepareContent {
    int64_t prepareStart;
    bool prepared;
    int64_t prepareUse;

} prepareContent;

static void prepareOnStatusChanged(int64_t oldStatus, int64_t newStatus, void *userData)
{
    auto *content = static_cast<prepareContent *>(userData);
    if (newStatus == PLAYER_PREPARINIT) {
        content->prepareStart = af_getsteady_ms();
    }
}

static void onPlayerPrepared(void *userData)
{
    auto *content = static_cast<prepareContent *>(userData);
    content->prepared = true;
    content->prepareUse = af_getsteady_ms() - content->prepareStart;
    AF_LOGD("prepare use %lld ms\n", af_getsteady_ms() - content->prepareStart);
}

static int onCallback(Cicada::MediaPlayer *player, void *arg)
{
    auto *content = static_cast<prepareContent *>(arg);
    if (content->prepared) {
        return -1;
    }
    return 0;
}

static int64_t prepareOnce()
{
    prepareContent content;
    content.prepareStart = -1;
    content.prepared = false;
    content.prepareUse = -1;
    playerListener listener{nullptr};
    listener.Prepared = onPlayerPrepared;
    listener.StatusChanged = prepareOnStatusChanged;
    listener.userData = &content;
    test_simple("https://alivc-demo-vod.aliyuncs.com/sv/34988cb9-17c9d023e6d/34988cb9-17c9d023e6d.mp4", nullptr, onCallback, &content,
                &listener);
    return content.prepareUse;
}

TEST(performance, prepare)
{
    globalSettings::getSetting().addResolve("alivc-demo-vod.aliyuncs.com:443", "27.128.214.222");
    const static int size = 20;
    int64_t timeCost[size];
    int64_t timeCost2[size];
    bool h2;
    for (int i = 0; i < size * 2; ++i) {
        h2 = (i % 2 == 0);
        if (!h2) {
            globalSettings::getSetting().setProperty("protected.network.http.http2", "OFF");
            timeCost[i / 2] = prepareOnce();
        } else {
            globalSettings::getSetting().setProperty("protected.network.http.http2", "ON");
            timeCost2[i / 2] = prepareOnce();
        }
    }
    int64_t sum = 0;
    for (int64_t i : timeCost) {
        sum += i;
        AF_LOGI("time cost is %lld\n", i);
    }
    AF_LOGI("avg time cost is %lld\n", sum / size);

    sum = 0;
    for (int64_t i : timeCost2) {
        sum += i;
        AF_LOGI("time cost2 is %lld\n", i);
    }
    AF_LOGI("avg time cost2 is %lld\n", sum / size);
}
typedef struct seekContent {
    bool playing;
    int64_t seekStart;
    int64_t seekEnd;
    bool running;
} seekContent;

static void seekOnStatusChanged(int64_t oldStatus, int64_t newStatus, void *userData)
{
    auto *content = static_cast<seekContent *>(userData);
    if (newStatus == PLAYER_PLAYING) {
        content->playing = true;
    }
}
void onSeekEnd(int64_t inCache, void *userData)
{
    if (inCache) {
        AF_LOGI("seek in cache\n");
        assert(0);
        return;
    }
    auto *content = static_cast<seekContent *>(userData);
    content->seekEnd = af_getsteady_ms();
}
static void seekTest(const float (&seekPos)[], int64_t (&seekSpend)[], int size)
{
    seekContent content;
    content.playing = false;
    content.seekStart = INT64_MIN;
    content.seekEnd = INT64_MIN;
    playerListener listener{nullptr};
    listener.StatusChanged = seekOnStatusChanged;
    listener.SeekEnd = onSeekEnd;
    listener.userData = &content;
    unique_ptr<MediaPlayer> player = unique_ptr<MediaPlayer>(new MediaPlayer());
#ifdef ENABLE_SDL
    CicadaSDLView view;

    view.view = getView();
    view.type = CicadaSDLViewType_SDL_WINDOW;
    player->SetView(&view);
#endif
    player->SetDataSource("https://alivc-demo-vod.aliyuncs.com/sv/34988cb9-17c9d023e6d/34988cb9-17c9d023e6d.mp4");
    player->SetAutoPlay(true);
    player->SetListener(listener);
    player->Prepare();
    int seekCount = 0;
    SDL_Event event;
    while (seekCount < size) {
        af_msleep(10);
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                break;
            }
        }
        if (!content.playing) {
            continue;
        }
        if (content.seekStart < 0) {
            content.seekStart = af_getsteady_ms();
            content.seekEnd = INT64_MIN;
            player->SeekTo((int64_t) (player->GetDuration() * seekPos[seekCount]), SEEK_MODE_INACCURATE);
        } else if (content.seekEnd < 0) {
            continue;
        } else {
            seekSpend[seekCount] = content.seekEnd - content.seekStart;
            seekCount++;
            content.seekStart = INT64_MIN;
        }
    }
}

TEST(performance, seek)
{
    globalSettings::getSetting().addResolve("alivc-demo-vod.aliyuncs.com:443", "27.128.214.222");
    const static int size = 20;
    float seekPos[size];
    float step = 1.0f / size;
    for (int i = 0; i < size; ++i) {
        seekPos[size - i - 1] = step * i;
    }
    int64_t seekSpend[size];
    int64_t seekSpend2[size];
#if 1
    globalSettings::getSetting().setProperty("protected.network.http.http2", "ON");
    seekTest(reinterpret_cast<float(&)[]>(seekPos), reinterpret_cast<int64_t(&)[]>(seekSpend2), size);
    globalSettings::getSetting().setProperty("protected.network.http.http2", "OFF");
    seekTest(reinterpret_cast<float(&)[]>(seekPos), reinterpret_cast<int64_t(&)[]>(seekSpend), size);
#else
    globalSettings::getSetting().setProperty("protected.network.http.http2", "OFF");
    seekTest(reinterpret_cast<float(&)[]>(seekPos), reinterpret_cast<int64_t(&)[]>(seekSpend), size);
    globalSettings::getSetting().setProperty("protected.network.http.http2", "ON");
    seekTest(reinterpret_cast<float(&)[]>(seekPos), reinterpret_cast<int64_t(&)[]>(seekSpend2), size);
#endif
    int64_t sum = 0;
    for (auto item : seekSpend) {
        AF_LOGI("seek spend %lld\n", item);
        sum += item;
    }
    AF_LOGI("avg seek spend is %lld\n", sum / size);
    sum = 0;
    for (auto item : seekSpend2) {
        AF_LOGI("seek2 spend %lld\n", item);
        sum += item;
    }
    AF_LOGI("avg seek2 spend is %lld\n", sum / size);
}
