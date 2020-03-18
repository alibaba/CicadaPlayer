//
// Created by pingkai on 2020/1/14.
//

#include "gtest/gtest.h"
#include <memory>
#include "tests/mediaPlayerTest.h"
#include <utils/timer.h>
#include "tests/player_command.h"
#include <vector>
#include <utils/AFUtils.h>

using namespace std;


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
//    log_set_level(AF_LOG_LEVEL_TRACE, 1);
    ignore_signal(SIGPIPE);
    return RUN_ALL_TESTS();
}

static void onStreamInfoGet(int64_t size, const void *msg, void *userData)
{
    StreamInfo **info = (StreamInfo **) msg;

    for (int i = 0; i < size; ++i) {
        AF_LOGD("get a %d type stream", info[i]->type);
    }
}

static void onStreamSwitchSuc(int64_t size, const void *msg, void *userData)
{
    StreamInfo *info = (StreamInfo *) msg;
    StreamType type = static_cast<StreamType>(size);
    //   ASSERT_TRUE(type == ST_TYPE_VIDEO);
}

bool prepared = false;

static void createTestCase_switchVideo(commandsCase &testCase)
{
    player_command cmd;
    int count = 5;
    int posDelta = 10000;
    cmd.mID = player_command::setLoop;
    cmd.timestamp = 0;
    cmd.arg0 = 1;
    std::unique_lock <std::mutex>lock(testCase.mMutex);
    testCase.mCommands.push_back(cmd);
    cmd.mID = player_command::selectStream;
    int64_t start_time = af_getsteady_ms();

    for (int i = 0; i < count; i++) {
        cmd.timestamp = i * posDelta + start_time;
        cmd.arg0 = i;
        testCase.mCommands.push_back(cmd);
    }
}

static void createTestCase_switchSubtitle(commandsCase &testCase)
{
    player_command cmd;
    int count = 2;
    int posDelta = 2000;
    cmd.mID = player_command::setLoop;
    cmd.timestamp = 0;
    cmd.arg0 = 1;
    std::unique_lock<std::mutex> lock(testCase.mMutex);
    testCase.mCommands.push_back(cmd);
    cmd.mID = player_command::selectStream;
    int64_t start_time = af_getsteady_ms();

    for (int i = 1; i <= count; i++) {
        cmd.timestamp = i * posDelta + start_time;
        cmd.arg0 = i;
        testCase.mCommands.push_back(cmd);
    }

    cmd.timestamp += 3 * posDelta;
    cmd.mID = player_command::setLoop;
    cmd.arg0 = 0;
    testCase.mCommands.push_back(cmd);
}

static void onPrepared_video(void *userData)
{
    prepared = true;
    commandsCase *testCase = static_cast<commandsCase *>(userData);
    player_command cmd;
    cmd.mID = player_command::start;
    cmd.timestamp = af_getsteady_ms();
    {
        std::unique_lock<std::mutex> lock(testCase->mMutex);
        testCase->mCommands.push_back(cmd);
    }
    createTestCase_switchVideo(*testCase);
    testCase->mExitOnEmpty = true;
}


static void onPrepared_subtitle(void *userData)
{
    prepared = true;
    commandsCase *testCase = static_cast<commandsCase *>(userData);
    player_command cmd;
    cmd.mID = player_command::start;
    cmd.timestamp = af_getsteady_ms();
    {
        std::unique_lock<std::mutex> lock(testCase->mMutex);
        testCase->mCommands.push_back(cmd);
    }
    createTestCase_switchSubtitle(*testCase);
    testCase->mExitOnEmpty = true;
}

TEST(switch_stream, video)
{
    std::vector<player_command> commands;
    commandsCase testCase(commands, false);
    playerListener listener{nullptr};
    listener.StreamInfoGet = onStreamInfoGet;
    listener.StreamSwitchSuc = onStreamSwitchSuc;
    listener.Prepared = onPrepared_video;
    listener.userData = &testCase;
    prepared = false;
    test_simple("https://alivc-demo-vod.aliyuncs.com/59f748948daa4438b42e42db755ae01e/9d44b2b86d334c6b9df649e35ad0240f.m3u8", nullptr,
                command_loop,
                &testCase, &listener, true);
}

TEST(switch_stream, subtitle)
{
    std::vector<player_command> commands;
    commandsCase testCase(commands, false);
    playerListener listener{nullptr};
    listener.StreamInfoGet = onStreamInfoGet;
    listener.StreamSwitchSuc = onStreamSwitchSuc;
    listener.Prepared = onPrepared_subtitle;
    listener.userData = &testCase;
    test_simple("https://alivc-demo-vod.aliyuncs.com/07563e259f544e69bc3e5454293fc06a/1bb2b7f0e164494a88874c4911c3cec0.m3u8", nullptr,
                command_loop,
                &testCase, &listener);
}

Cicada::MediaPlayer *g_player = nullptr;

static int ExtSubtitleOnCallback(Cicada::MediaPlayer *player, void *arg)
{
    g_player = player;
    player->AddExtSubtitle(
        "https://alivc-demo-vod.aliyuncs.com/07563e259f544e69bc3e5454293fc06a/subtitles/cn/c8d7d959e85977bedf8a61dd25f85583.vtt");
    player->AddExtSubtitle(
        "https://alivc-demo-vod.aliyuncs.com/07563e259f544e69bc3e5454293fc06a/subtitles/en-us/c8d7d959e85977bedf8a61dd25f85583.vtt");
    return 0;
}

void onSubtitleExtAdd(int64_t index, const void *errorMsg, void *userData)
{
    ASSERT_TRUE(index > 0);
    commandsCase *testCase = static_cast<commandsCase *>(userData);
    g_player->SelectExtSubtitle(index, true);
    player_command cmd;
    cmd.mID = player_command::setLoop;
    cmd.arg0 = 0;
    cmd.timestamp = af_getsteady_ms() + 10000;
    std::unique_lock<std::mutex> lock(testCase->mMutex);
    testCase->mCommands.push_back(cmd);
    testCase->mExitOnEmpty = true;
}

static void onSubtitleShow(int64_t index, int64_t size, const void *buffer, void *userData)
{
}
static void onSubtitleHide(int64_t index, int64_t size, const void *buffer, void *userData)
{
}
TEST(switch_stream, ExtSubtitle)
{
    std::vector<player_command> commands;
    commandsCase testCase(commands, false);
    playerListener listener{nullptr};
    listener.StreamInfoGet = onStreamInfoGet;
    listener.StreamSwitchSuc = onStreamSwitchSuc;
    listener.Prepared = onPrepared_subtitle;
    listener.SubtitleExtAdd = onSubtitleExtAdd;
    listener.SubtitleShow = onSubtitleShow;
    listener.SubtitleHide = onSubtitleHide;
    listener.userData = &testCase;
    test_simple(
        "https://alivc-demo-vod.aliyuncs.com/07563e259f544e69bc3e5454293fc06a/video/1bb2b7f0e164494a88874c4911c3cec0-85333fe978bd741193df2c08e697757b-video-ld.m3u8",
        ExtSubtitleOnCallback,
        command_loop,
        &testCase, &listener);
}
