//
// Created by moqi on 2020/1/10.
//


#include "gtest/gtest.h"
#include <memory>
#include "mediaPlayerTest.h"

using namespace std;
int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
//    log_set_level(AF_LOG_LEVEL_TRACE, 1);
    return RUN_ALL_TESTS();
}




TEST(play, mp4)
{
    string url = "http://player.alicdn.com/video/aliyunmedia.mp4";
    test_player(url);
}
