//
// Created by moqi on 2020/1/14.
//

#include "gtest/gtest.h"
#include <memory>
#include "tests/mediaPlayerTest.h"
#include <utils/timer.h>
#include "tests/player_command.h"
#include <vector>
#include <data_source/cachedSource.h>
#include <data_source/SourceReader.h>
#include <utils/AFUtils.h>


using namespace std;
using namespace Cicada;

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
//    log_set_level(AF_LOG_LEVEL_TRACE, 1);
    ignore_signal(SIGPIPE);
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
    return 0;
}

TEST(cache, file)
{
    test_simple("http://player.alicdn.com/video/aliyunmedia.mp4", OnCreate, simple_loop,
                nullptr, nullptr);
}

SourceReader *g_reader = nullptr;

int readCallBack(void *arg, uint8_t *buffer, int size)
{
    AF_TRACE;
    auto *reader = static_cast<SourceReader *>(arg);
    return reader->read(buffer, size);
}

int64_t seekCallBack(void *arg, int64_t offset, int whence)
{
    AF_TRACE;
    auto *reader = static_cast<SourceReader *>(arg);
    return reader->seek(offset, whence);
}

static int cachedSourceOnCreate(Cicada::MediaPlayer *player, void *arg)
{
    shared_ptr<cachedSource> source = make_shared<cachedSource>("http://player.alicdn.com/video/aliyunmedia.mp4", 0);
    g_reader = new SourceReader(source);
    player->setBitStreamCb(readCallBack, seekCallBack, g_reader);
    //  log_set_level(AF_LOG_LEVEL_DEBUG, 1);
    return 0;
}

TEST(cache, cachedSource)
{
    test_simple("", cachedSourceOnCreate, simple_loop, nullptr, nullptr);
    delete g_reader;
    g_reader = nullptr;
}

