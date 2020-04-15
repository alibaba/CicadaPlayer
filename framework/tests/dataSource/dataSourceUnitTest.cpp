//
// Created by moqi on 2019/11/14.
//

#include "gtest/gtest.h"
#include <data_source/dataSourcePrototype.h>
#include <memory>
#include <utils/timer.h>
#include <utils/globalSettings.h>
#include <utils/frame_work_log.h>
#include <utils/CicadaJSON.h>
#include <utils/errors/framework_error.h>
#include <utils/AFUtils.h>

using namespace std;
using namespace Cicada;

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    log_set_level(AF_LOG_LEVEL_TRACE, 1);
    ignore_signal(SIGPIPE);
    return RUN_ALL_TESTS();
}

TEST(protocol, http)
{
    string url = "http://player.alicdn.com/video/aliyunmedia.mp4";
    unique_ptr<IDataSource> source = unique_ptr<IDataSource>(dataSourcePrototype::create(url));
    ASSERT_NE(source, nullptr);
    int ret = source->Open(0);
    ASSERT_GE(ret, 0);
    int64_t size = source->Seek(0, SEEK_SIZE);
    ASSERT_EQ(size, 146057805);
}


TEST(protocol, https)
{
    string url = "https://player.alicdn.com/video/aliyunmedia.mp4";
    unique_ptr<IDataSource> source = unique_ptr<IDataSource>(dataSourcePrototype::create(url));
    ASSERT_NE(source, nullptr);
    int ret = source->Open(0);
    ASSERT_GE(ret, 0);
    int64_t size = source->Seek(0, SEEK_SIZE);
    ASSERT_EQ(size, 146057805);
}

TEST(protocol, file)
{
    unlink("test");
    FILE *file = fopen("test", "w");
    const char *buffer = "test file";
    fwrite(buffer, strlen(buffer) + 1, 1, file);
    fclose(file);
    string url = "test";
    unique_ptr<IDataSource> source = unique_ptr<IDataSource>(dataSourcePrototype::create(url));
    ASSERT_NE(source, nullptr);
    int ret = source->Open(0);
    ASSERT_GE(ret, 0);
    int64_t size = source->Seek(0, SEEK_SIZE);
    ASSERT_EQ(size, strlen(buffer) + 1);
    char *obuffer = static_cast<char *>(malloc(strlen(buffer) + 1));
    source->Read(obuffer, strlen(buffer) + 1);
    ASSERT_EQ(memcmp(buffer, obuffer, strlen(buffer) + 1), 0);
    free(obuffer);
}

TEST(dns, https)
{
    // https://ip.tool.chinaz.com/
    // https://dnschecker.org/
    string url = "https://player.alicdn.com/video/aliyunmedia.mp4";
    //  string ip = "120.77.195.15";
    string ip = "47.246.17.253";
    globalSettings::getSetting()->addResolve("player.alicdn.com:443", ip);
    unique_ptr<IDataSource> source = unique_ptr<IDataSource>(dataSourcePrototype::create(url));
    int ret = source->Open(0);
    ASSERT_GE(ret, 0);
    string info = source->GetOption("connectInfo");
    CicadaJSONItem item(info);
    string host = item.getString("ip");
    ASSERT_TRUE(host == ip);
//   AF_LOGD("%s", info.c_str());
    globalSettings::getSetting()->removeResolve("player.alicdn.com:443", ip);
}

TEST(dns, http)
{
    // https://ip.tool.chinaz.com/
    // https://dnschecker.org/
    string url = "http://player.alicdn.com/video/aliyunmedia.mp4";
    //  string ip = "120.77.195.15";
    string ip = "47.246.17.253";
    globalSettings::getSetting()->addResolve("player.alicdn.com:80", ip);
    unique_ptr<IDataSource> source = unique_ptr<IDataSource>(dataSourcePrototype::create(url));
    int ret = source->Open(0);
    ASSERT_GE(ret, 0);
    string info = source->GetOption("connectInfo");
    CicadaJSONItem item(info);
    string host = item.getString("ip");
    ASSERT_TRUE(host == ip);
    globalSettings::getSetting()->removeResolve("player.alicdn.com:80", ip);
}

TEST(http, 404)
{
    string url = "https://img.alicdn.com/tfs/TB1DaGEcnvI8KJjSspjXXcgjXXa-220-781.png";
    unique_ptr<IDataSource> source = unique_ptr<IDataSource>(dataSourcePrototype::create(url));
    int ret = source->Open(0);
    ASSERT_EQ(ret, gen_framework_errno(error_class_network, network_errno_http_404));
}

TEST(api, read)
{
    string url = "https://player.alicdn.com/video/aliyunmedia.mp4";
    void *buf = malloc(1024 * 16);
    unique_ptr<IDataSource> source = unique_ptr<IDataSource>(dataSourcePrototype::create(url));
    int ret = source->Open(0);
    ASSERT_GE(ret, 0);
    int index = 0;

    do {
        ret = source->Read(buf, 1024 * 16);

        if (index ++ > 10) {
            break;
        }
    } while (ret > 0);

    free(buf);
}

TEST(api, seek)
{
    string url = "https://player.alicdn.com/video/aliyunmedia.mp4";
    void *buf = malloc(1024);
    unique_ptr<IDataSource> source = unique_ptr<IDataSource>(dataSourcePrototype::create(url));
    int ret = source->Open(0);
    ASSERT_GE(ret, 0);
    int index = 0;

    do {
        ret = source->Read(buf, 1024);
        source->Seek(1000, SEEK_CUR);

        if (index ++ > 10) {
            break;
        }
    } while (ret > 0);

    free(buf);
}

//TEST(api, slowRead) {
//    string url = "https://player.alicdn.com/video/aliyunmedia.mp4";
//    void *buf = malloc(1024 * 16);
//    unique_ptr<IDataSource> source = unique_ptr<IDataSource>(dataSourcePrototype::create(url));
//    int ret = source->Open(0);
//    ASSERT_GE(ret, 0);
//
//    int index = 0;
//
//    do {
//        af_msleep(2000);
//        ret = source->Read(buf, 1024 * 16 -1);
//        if (index++ > 10)
//            break;
//    } while (ret > 0);
//}

TEST(http, keep_alive)
{
    string url = "https://player.alicdn.com/video/aliyunmedia.mp4";
    void *buf = malloc(1024);
    unique_ptr<IDataSource> source = unique_ptr<IDataSource>(dataSourcePrototype::create(url));
    int ret = source->Open(0);
    ASSERT_GE(ret, 0);
    source->Seek(1024, SEEK_CUR);
    ret = source->Read(buf, 1024);
    source->Open(url);
    ASSERT_GE(ret, 0);
    source->Seek(-1024, SEEK_END);

    do {
        ret = source->Read(buf, 1024);
    } while (ret > 0);

    free(buf);
}

#include "ipList.h"

TEST(https, ipList)
{
    int size = sizeof(IpList) / sizeof(IpList[0]);

    if (size == 0) {
        return;
    }

    for (int i = 0; i < size; ++i) {
        globalSettings::getSetting()->addResolve(ipListHots, IpList[i]);

        if (i > 0) {
            globalSettings::getSetting()->removeResolve(ipListHots, IpList[i - 1]);
        }

        unique_ptr<IDataSource> source = unique_ptr<IDataSource>(dataSourcePrototype::create(ipListUrl));
        int ret = source->Open(0);
        ASSERT_GE(ret, 0);
        ASSERT_EQ(source->Seek(0, SEEK_SIZE), ipListFileSize);
    }
}
