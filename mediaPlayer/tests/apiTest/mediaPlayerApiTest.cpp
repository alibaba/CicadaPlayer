//
// Created by pingkai on 2020/1/13.
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

TEST(cmd, volume)
{
    std::vector<player_command> commands;
    player_command cmd;
    int posDelta = 1000;
    commands.reserve(26);
    cmd.mID = player_command::player_command_loop;
    cmd.timestamp = 0;
    cmd.arg0 = 1;
    commands.push_back(cmd);
    cmd.mID = player_command::player_command_volume;
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


    test_simple("http://player.alicdn.com/video/aliyunmedia.mp4", command_loop,
                &testCase, nullptr);
}

TEST(cmd, speed)
{
    std::vector<player_command> commands;
    player_command cmd;
    int posDelta = 1000;
    commands.reserve(26);
    cmd.mID = player_command::player_command_loop;
    cmd.timestamp = 0;
    cmd.arg0 = 1;
    commands.push_back(cmd);
    cmd.mID = player_command::player_command_speed;
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

    test_simple("http://player.alicdn.com/video/aliyunmedia.mp4", command_loop,
                &testCase, nullptr);
}

