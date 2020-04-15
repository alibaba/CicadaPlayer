//
// Created by moqi on 2020/1/10.
//


#include "gtest/gtest.h"
#include <memory>
#include "tests/mediaPlayerTest.h"
#include <utils/timer.h>
#include "tests/player_command.h"
#include <vector>
#include "../mediaPlayerTest.h"
#include <utils/AFUtils.h>
using namespace std;

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
//    log_set_level(AF_LOG_LEVEL_TRACE, 1);
    ignore_signal(SIGPIPE);
    return RUN_ALL_TESTS();
}

#ifdef ENABLE_SDL

    #include <SDL2/SDL.h>
    #include <utils/timer.h>

#endif


TEST(play, mp4)
{
    test_simple("http://player.alicdn.com/video/aliyunmedia.mp4", nullptr, simple_loop, nullptr, nullptr);
}

TEST(play, hls)
{
    test_simple(
        "https://alivc-demo-vod.aliyuncs.com/ddb0c76ce153450081cd4c45118371a7/d30995ad97bc4643bf0a8c4cedd0c81f-e16b4635a4cb03424234c3a3d0e7f7e1-sd.m3u8",
        nullptr,
        simple_loop, nullptr, nullptr);
}

TEST(HLS, master)
{
    test_simple("https://alivc-demo-vod.aliyuncs.com/59f748948daa4438b42e42db755ae01e/9d44b2b86d334c6b9df649e35ad0240f.m3u8", nullptr,
                simple_loop,
                nullptr, nullptr);
}

TEST(HLS, AES_128)
{
    test_simple(
        "https://alivc-demo-vod.aliyuncs.com/d2c89d7210d443109434685f45ed607b/45ed0cccd8092bf25ee33764b5a52be4-sd-encrypt-stream.m3u8",
        nullptr,
        simple_loop, nullptr, nullptr);
}

TEST(HLS, subtitle)
{
    test_simple("https://alivc-demo-vod.aliyuncs.com/07563e259f544e69bc3e5454293fc06a/1bb2b7f0e164494a88874c4911c3cec0.m3u8", nullptr,
                simple_loop,
                nullptr, nullptr);
}

TEST(HLS, live)
{
    test_simple("http://qt1.alivecdn.com/timeline/testshift.m3u8?auth_key=1594730859-0-0-b71fd57c57a62a3c2b014f24ca2b9da3", nullptr,
                simple_loop,
                nullptr, nullptr);
}

Cicada::MediaPlayer *g_player = nullptr;

static int SubTitleEXTOnCreate(Cicada::MediaPlayer *player, void *arg)
{
    g_player = player;
    return 0;
}

void SubTitleEXTOnPrepared(void *userData)
{
    g_player->AddExtSubtitle(
        "https://alivc-demo-vod.aliyuncs.com/07563e259f544e69bc3e5454293fc06a/subtitles/cn/c8d7d959e85977bedf8a61dd25f85583.vtt");
    g_player->AddExtSubtitle(
        "https://alivc-demo-vod.aliyuncs.com/07563e259f544e69bc3e5454293fc06a/subtitles/en-us/c8d7d959e85977bedf8a61dd25f85583.vtt");
}

static void onSubtitleExtAdd(int64_t index, const void *errorMsg, void *userData)
{
    ASSERT_TRUE(index > 0);
    g_player->SelectExtSubtitle(index, true);
}

static void onSubtitleShow(int64_t index, int64_t size, const void *buffer, void *userData)
{
}
static void onSubtitleHide(int64_t index, int64_t size, const void *buffer, void *userData)
{
}

TEST(SubTitle, EXT)
{
    playerListener listener{nullptr};
    listener.Prepared = SubTitleEXTOnPrepared;
    listener.SubtitleExtAdd = onSubtitleExtAdd;
    listener.SubtitleShow = onSubtitleShow;
    listener.SubtitleHide = onSubtitleHide;
    string url =
        "https://alivc-demo-vod.aliyuncs.com/07563e259f544e69bc3e5454293fc06a/video/1bb2b7f0e164494a88874c4911c3cec0-85333fe978bd741193df2c08e697757b-video-ld.m3u8";
    test_simple(url, SubTitleEXTOnCreate, simple_loop, nullptr, &listener);
}
