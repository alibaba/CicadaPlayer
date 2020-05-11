//
// Created by pingkai on 2020/1/13.
//

#include "tests/mediaPlayerTest.h"
#include "tests/player_command.h"
#include "gtest/gtest.h"
#include <memory>
#include <utils/AFUtils.h>
#include <utils/mediaFrame.h>
#include <utils/timer.h>
#include <vector>

#ifdef __APPLE__
#include <base/media/PBAFFrame.h>
#endif

using namespace std;

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
//    log_set_level(AF_LOG_LEVEL_TRACE, 1);
    ignore_signal(SIGPIPE);
    return RUN_ALL_TESTS();
}

TEST(cmd, volume)
{
    std::vector<player_command> commands;
    player_command cmd;
    int posDelta = 1000;
    commands.reserve(26);
    cmd.mID = player_command::setLoop;
    cmd.timestamp = 0;
    cmd.arg0 = 1;
    commands.push_back(cmd);
    cmd.mID = player_command::setVolume;
    int64_t start_time = af_getsteady_ms();

    for (int i = 0; i <= 10; i++) {
        cmd.timestamp = i * posDelta + start_time;
        cmd.arg0 = 10 + i;
        commands.push_back(cmd);
    }

    start_time += 11 * posDelta;

    for (int i = 0; i <= 20; i++) {
        cmd.timestamp = i * posDelta + start_time;
        cmd.arg0 = 20 - i;
        commands.push_back(cmd);
    }

    commandsCase testCase(commands, true);
    test_simple("http://player.alicdn.com/video/aliyunmedia.mp4", nullptr, command_loop,
                &testCase, nullptr);
}

TEST(cmd, speed)
{
    std::vector<player_command> commands;
    player_command cmd;
    int posDelta = 1000;
    commands.reserve(26);
    cmd.mID = player_command::setLoop;
    cmd.timestamp = 0;
    cmd.arg0 = 1;
    commands.push_back(cmd);
    cmd.mID = player_command::setSpeed;
    int64_t start_time = af_getsteady_ms();

    for (int i = 0; i <= 10; i++) {
        cmd.timestamp = i * posDelta + start_time;
        cmd.arg0 = 10 + i;
        commands.push_back(cmd);
    }

    start_time += 11 * posDelta;

    for (int i = 0; i <= 15; i++) {
        cmd.timestamp = i * posDelta + start_time;
        cmd.arg0 = 20 - i;
        commands.push_back(cmd);
    }

    commandsCase testCase(commands, true);
    test_simple("http://player.alicdn.com/video/aliyunmedia.mp4", nullptr, command_loop,
                &testCase, nullptr);
}

TEST(cmd, backGround)
{
    std::vector<player_command> commands;
    player_command cmd;
    int posDelta = 5000;
    commands.reserve(4);
    cmd.mID = player_command::setLoop;
    cmd.timestamp = 0;
    cmd.arg0 = 1;
    commands.push_back(cmd);
    cmd.mID = player_command::backGround;
    int64_t start_time = af_getsteady_ms() + 2000;

    for (int i = 0; i < 4; i++) {
        cmd.timestamp = i * posDelta + start_time;
        cmd.arg0 = (i + 1) % 2;
        commands.push_back(cmd);
    }

    commandsCase testCase(commands, true);
    test_simple("http://player.alicdn.com/video/aliyunmedia.mp4", nullptr, command_loop,
                &testCase, nullptr);
}

static bool g_autoPlay = false;

static void onAutoPlayStart(void *userData)
{
    g_autoPlay = true;
}
TEST(cmd, autoPlay)
{
    playerListener listener{nullptr};
    g_autoPlay = false;
    listener.AutoPlayStart = onAutoPlayStart;
    test_simple("http://player.alicdn.com/video/aliyunmedia.mp4", nullptr, simple_loop,
                nullptr, &listener);
    assert(g_autoPlay);
}

static void onCompletion(void *userData)
{
    commandsCase *testCase = static_cast<commandsCase *>(userData);
    testCase->mExitOnEmpty = true;
}

TEST(event, EOS)
{
    std::vector<player_command> commands;
    player_command cmd;
    commands.reserve(4);
    cmd.mID = player_command::seek;
    cmd.timestamp = af_getsteady_ms() + 2000;
    cmd.arg0 = (4 * 60 + 15) * 1000;
    commands.push_back(cmd);
    commandsCase testCase(commands, false);
    playerListener listener{nullptr};
    listener.Completion = onCompletion;
    listener.userData = &testCase;
    test_simple("http://player.alicdn.com/video/aliyunmedia.mp4", nullptr, command_loop,
                &testCase, &listener);
    ASSERT_TRUE(testCase.mExitOnEmpty);
}


Cicada::MediaPlayer *apiPlayer = nullptr;

static void pictureApiOnPrepared(void *arg)
{
    ASSERT_TRUE(apiPlayer);
    apiPlayer->CaptureScreen();
    apiPlayer->SetMirrorMode(MIRROR_MODE_HORIZONTAL);
    MirrorMode mode = apiPlayer->GetMirrorMode();
    apiPlayer->SetRotateMode(ROTATE_MODE_0);
    apiPlayer->GetRotateMode();
    apiPlayer->SetScaleMode(SM_FIT);
    apiPlayer->GetScaleMode();
}

static int pictureApiOnCallback(Cicada::MediaPlayer *player, void *arg)
{
    apiPlayer = player;
    return 0;
}

TEST(coverag, pictureApi)
{
    playerListener listener{nullptr};
    listener.Prepared = pictureApiOnPrepared;
    test_simple("http://player.alicdn.com/video/aliyunmedia.mp4", pictureApiOnCallback, simple_loop,
                nullptr, &listener);
}

static void getApiOnPrepared(void *arg)
{
    ASSERT_TRUE(apiPlayer);
    apiPlayer->GetVideoHeight();
    apiPlayer->GetVideoWidth();
    apiPlayer->GetSpeed();
    apiPlayer->GetVolume();
    apiPlayer->GetCurrentPosition();
    apiPlayer->GetAnalyticsCollector();
    apiPlayer->GetBufferedPosition();
    apiPlayer->GetCurrentStreamIndex(ST_TYPE_VIDEO);
    apiPlayer->GetMasterClockPts();
    apiPlayer->GetDuration();
    apiPlayer->GetConfig();
    apiPlayer->GetScaleMode();
    apiPlayer->GetVideoRotation();
    apiPlayer->IsAutoPlay();
    apiPlayer->IsLoop();
    apiPlayer->IsMuted();
    apiPlayer->GetPropertyString(PROPERTY_KEY_RESPONSE_INFO);
    apiPlayer->GetPropertyString(PROPERTY_KEY_CONNECT_INFO);
    apiPlayer->GetPropertyString(PROPERTY_KEY_OPEN_TIME_STR);
    apiPlayer->GetPropertyString(PROPERTY_KEY_PROBE_STR);
    apiPlayer->GetPropertyString(PROPERTY_KEY_VIDEO_BUFFER_LEN);
    apiPlayer->GetPropertyString(PROPERTY_KEY_DELAY_INFO);
    apiPlayer->GetPropertyString(PROPERTY_KEY_REMAIN_LIVE_SEG);
    apiPlayer->GetPropertyString(PROPERTY_KEY_NETWORK_IS_CONNECTED);
    Stream_meta meta{};
    apiPlayer->GetCurrentStreamMeta(&meta, ST_TYPE_VIDEO);
    releaseMeta(&meta);
}

TEST(coverag, getApi)
{
    playerListener listener{nullptr};
    listener.Prepared = getApiOnPrepared;
    test_simple("http://player.alicdn.com/video/aliyunmedia.mp4", pictureApiOnCallback, simple_loop,
                nullptr, &listener);
}

static int setApiOnCallback(Cicada::MediaPlayer *player, void *arg)
{
    Cicada::MediaPlayerConfig config;
    config.startBufferDuration = 500;
    config.highBufferDuration = 5000;
    config.bEnableTunnelRender = false;
    config.httpProxy = "";
    config.maxBufferDuration = 60 * 1000;
    player->SetConfig(&config);
    return 0;
}

TEST(config, set)
{
    test_simple("http://player.alicdn.com/video/aliyunmedia.mp4", setApiOnCallback, simple_loop,
                nullptr, nullptr);
}

bool OnRenderFrame(void *userData, IAFFrame *frame)
{
    if (frame->getType() == IAFFrame::FrameTypeVideo) {
        switch (frame->getInfo().format) {
            case AF_PIX_FMT_APPLE_PIXEL_BUFFER: {
                AF_LOGD("get a apple pixel buffer");
#ifdef __APPLE__
                auto *ppBFrame = dynamic_cast<PBAFFrame *>(frame);
                ppBFrame->getPixelBuffer();
#endif
                break;
            }
            case AF_PIX_FMT_YUV420P:
                AF_LOGD("get a yuv 420p");
                frame->getData();
                frame->getLineSize();
                break;
            default:
                break;
        }
    }
    return false;
}

static int setRenderCbOnCallback(Cicada::MediaPlayer *player, void *arg)
{
    player->SetOnRenderFrameCallback(OnRenderFrame, nullptr);
    return 0;
}

TEST(api, renderCb)
{
    test_simple("http://player.alicdn.com/video/aliyunmedia.mp4", setRenderCbOnCallback, simple_loop, nullptr, nullptr);
}
