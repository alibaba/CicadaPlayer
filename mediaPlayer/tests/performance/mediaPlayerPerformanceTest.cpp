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

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    log_set_level(AF_LOG_LEVEL_DEBUG, 1);
    log_enable_color(1);
    //    ignore_signal(SIGPIPE);
    return RUN_ALL_TESTS();
}

typedef struct mainContent {
    int64_t prepareStart;
    bool prepared;
    int64_t prepareUse;

} mainContent;

static void onStatusChanged(int64_t oldStatus, int64_t newStatus, void *userData)
{
    auto *content = static_cast<mainContent *>(userData);
    if (newStatus == PLAYER_PREPARINIT) {
        content->prepareStart = af_getsteady_ms();
    }
}

static void onPlayerPrepared(void *userData)
{
    auto *content = static_cast<mainContent *>(userData);
    content->prepared = true;
    content->prepareUse = af_getsteady_ms() - content->prepareStart;
    AF_LOGD("prepare use %lld ms\n", af_getsteady_ms() - content->prepareStart);
}

static int onCallback(Cicada::MediaPlayer *player, void *arg)
{
    auto *content = static_cast<mainContent *>(arg);
    if (content->prepared) {
        return -1;
    }
    return 0;
}

static int64_t prepareOnce()
{
    mainContent content;
    content.prepareStart = -1;
    content.prepared = false;
    content.prepareUse = -1;
    playerListener listener{nullptr};
    listener.Prepared = onPlayerPrepared;
    listener.StatusChanged = onStatusChanged;
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
