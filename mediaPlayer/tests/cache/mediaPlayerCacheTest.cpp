//
// Created by moqi on 2020/1/14.
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

static int OnCreate(Cicada::MediaPlayer *player, void *arg)
{
    CacheConfig config{};
    config.mEnable = true;
    config.mCacheDir = "/tmp/Cicada";
    config.mMaxDirSizeMB = 1000;
    config.mMaxDurationS = 1000;

    player->SetCacheConfig(config);
}

TEST(cache, cache)
{
    test_simple("http://player.alicdn.com/video/aliyunmedia.mp4", OnCreate, simple_loop,
                nullptr, nullptr);

}

