//
// Created by pingkai on 2020/1/13.
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

static int seekEndTimes = 0;
static int seekEndPos = 0;

void onSeekEnd(int64_t position, void *userData)
{
    commandsCase *testCase = static_cast<commandsCase *>(userData);

    if (testCase->mCommands.empty()) {
        testCase->mExitOnEmpty = true;
    }

    seekEndTimes++;
    seekEndPos = position;
}

void onLoadingProgress(int64_t position, void *userData)
{
}

void onLoadingStart(void *userData)
{
}

void onLoadingEnd(void *userData)
{
}

TEST(cmd, seek)
{
    std::vector<player_command> commands;
    player_command cmd;
    int start = 0;
    int count = 10;
    int posDelta = 2000;
    int seekDelta = 10 * 1000;
    commands.reserve(count + 1);
    cmd.mID = player_command::setLoop;
    cmd.timestamp = 0;
    cmd.arg0 = 1;
    commands.push_back(cmd);
    cmd.mID = player_command::seek;
    int64_t start_time = af_getsteady_ms();

    for (int i = 0; i < count; i++) {
        cmd.timestamp = i * posDelta + start_time;
        cmd.arg0 = seekDelta;
        commands.push_back(cmd);
    }

    commandsCase testCase(commands, false);
    playerListener listener{nullptr};
    listener.SeekEnd = onSeekEnd;
    listener.LoadingProgress = onLoadingProgress;
    listener.LoadingStart = onLoadingStart;
    listener.LoadingEnd = onLoadingEnd;
    listener.userData = &testCase;
    test_simple("http://player.alicdn.com/video/aliyunmedia.mp4", nullptr, command_loop,
                &testCase, &listener);
    ASSERT_LE(seekEndTimes, count);
}

