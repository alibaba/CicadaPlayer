//
// Created by moqi on 2020/1/10.
//


#include "gtest/gtest.h"
#include <memory>
#include "mediaPlayerTest.h"
#include <utils/timer.h>

using namespace std;

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
//    log_set_level(AF_LOG_LEVEL_TRACE, 1);
    return RUN_ALL_TESTS();
}

#ifdef ENABLE_SDL

#include <sdl2/sdl.h>
#include <utils/timer.h>

#endif

static int g_play_time_s = 10;

static int simple_loop(Cicada::MediaPlayer *player)
{
#ifdef ENABLE_SDL
    int64_t start = af_getsteady_ms();
    SDL_Event event;
    while (true) {
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                break;
            }
        }
        af_msleep(10);
        if (af_getsteady_ms() - start > g_play_time_s*1000){
            break;
        }
    }
#else
    af_msleep(g_play_time_s *1000);
#endif
    return -1;
}

TEST(play, mp4)
{
    test_player("http://player.alicdn.com/video/aliyunmedia.mp4", simple_loop);
}

TEST(play, hls)
{
    test_player(
            "https://alivc-demo-vod.aliyuncs.com/ddb0c76ce153450081cd4c45118371a7/d30995ad97bc4643bf0a8c4cedd0c81f-e16b4635a4cb03424234c3a3d0e7f7e1-sd.m3u8",
            simple_loop);
}

TEST(HLS, master)
{
    test_player("https://alivc-demo-vod.aliyuncs.com/59f748948daa4438b42e42db755ae01e/9d44b2b86d334c6b9df649e35ad0240f.m3u8", simple_loop);
}

TEST(HLS, AES_128)
{
    test_player(
            "https://alivc-demo-vod.aliyuncs.com/d2c89d7210d443109434685f45ed607b/45ed0cccd8092bf25ee33764b5a52be4-sd-encrypt-stream.m3u8",
            simple_loop);
}

TEST(HLS, subtitle)
{
    test_player("https://alivc-demo-vod.aliyuncs.com/07563e259f544e69bc3e5454293fc06a/1bb2b7f0e164494a88874c4911c3cec0.m3u8", simple_loop);
}

TEST(HLS, live)
{
    test_player("http://qt1.alivecdn.com/timeline/testshift.m3u8?auth_key=1594730859-0-0-b71fd57c57a62a3c2b014f24ca2b9da3", simple_loop);

}