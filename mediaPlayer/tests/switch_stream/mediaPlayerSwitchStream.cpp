//
// Created by pingkai on 2020/1/14.
//

#include "gtest/gtest.h"
#include <memory>
#include "tests/mediaPlayerTest.h"
#include <utils/timer.h>
#include "tests/player_command.h"
#include <vector>

using namespace std;

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
//    log_set_level(AF_LOG_LEVEL_TRACE, 1);
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
    testCase.mCommands.reserve(count + 1);
    cmd.mID = player_command::setLoop;
    cmd.timestamp = 0;
    cmd.arg0 = 1;
    testCase.mCommands.push_back(cmd);
    cmd.mID = player_command::selectStream;
    int64_t start_time = af_getsteady_ms();
    std::unique_lock <std::mutex>lock(testCase.mMutex);
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
    testCase.mCommands.reserve(count + 1);
    cmd.mID = player_command::setLoop;
    cmd.timestamp = 0;
    cmd.arg0 = 1;
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
    std::unique_lock <std::mutex>lock(testCase.mMutex);
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
        std::unique_lock <std::mutex>lock(testCase->mMutex);
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
