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
        AF_LOGD("get a %d type stream", info[i]->videoBandwidth);
    }


}

static void onStreamSwitchSuc(int64_t size, const void *msg, void *userData)
{
    StreamInfo *info = (StreamInfo *) msg;
    StreamType type = static_cast<StreamType>(size);
    ASSERT_TRUE(type == ST_TYPE_VIDEO);


}

TEST(switch_stream, video)
{
    std::vector<player_command> commands;
    player_command cmd;
    int start = 0;
    int count = 5;
    int posDelta = 10000;
    commands.reserve(count + 1);
    cmd.mID = player_command::player_command_loop;
    cmd.timestamp = 0;
    cmd.arg0 = 1;
    commands.push_back(cmd);
    cmd.mID = player_command::player_command_selectStream;
    int64_t start_time = af_getsteady_ms();
    for (int i = 0; i < count; i++) {
        cmd.timestamp = i * posDelta + start_time;
        cmd.arg0 = i;
        commands.push_back(cmd);
    }
    commandsCase testCase(commands, true);
    playerListener listener{nullptr};
    listener.StreamInfoGet = onStreamInfoGet;
    listener.StreamSwitchSuc = onStreamSwitchSuc;
    test_simple("https://alivc-demo-vod.aliyuncs.com/59f748948daa4438b42e42db755ae01e/9d44b2b86d334c6b9df649e35ad0240f.m3u8", command_loop,
                &testCase, &listener);
}