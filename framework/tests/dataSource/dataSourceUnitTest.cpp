//
// Created by moqi on 2019/11/14.
//

#include "gtest/gtest.h"
#include <data_source/curl/curl_data_source.h>
#include <data_source/dataSourcePrototype.h>
#include <memory>
#include <utils/AFUtils.h>
#include <utils/CicadaJSON.h>
#include <utils/errors/framework_error.h>
#include <utils/frame_work_log.h>
#include <utils/globalSettings.h>
#include <utils/property.h>
#include <utils/timer.h>

using namespace std;
using namespace Cicada;

int main(int argc, char **argv)
{
    setProperty("protected.network.http.http2", "ON");
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
    string ip = "140.249.89.252";
    globalSettings::getSetting().addResolve("player.alicdn.com:443", ip);
    unique_ptr<IDataSource> source = unique_ptr<IDataSource>(dataSourcePrototype::create(url));
    int ret = source->Open(0);
    ASSERT_GE(ret, 0);
    string info = source->GetOption("connectInfo");
    CicadaJSONItem item(info);
    string host = item.getString("ip");
    ASSERT_TRUE(host == ip);
//   AF_LOGD("%s", info.c_str());
    globalSettings::getSetting().removeResolve("player.alicdn.com:443", ip);
}

TEST(dns, http)
{
    // https://ip.tool.chinaz.com/
    // https://dnschecker.org/
    string url = "http://player.alicdn.com/video/aliyunmedia.mp4";
    //  string ip = "120.77.195.15";
    string ip = "140.249.89.252";
    globalSettings::getSetting().addResolve("player.alicdn.com:80", ip);
    unique_ptr<IDataSource> source = unique_ptr<IDataSource>(dataSourcePrototype::create(url));
    int ret = source->Open(0);
    ASSERT_GE(ret, 0);
    string info = source->GetOption("connectInfo");
    CicadaJSONItem item(info);
    string host = item.getString("ip");
    ASSERT_TRUE(host == ip);
    globalSettings::getSetting().removeResolve("player.alicdn.com:80", ip);
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

TEST(http, post)
{
    string url = "https://ptsv2.com/t/50oow-1602229322";
    CurlDataSource source(url);
    uint8_t c = 'c';
    source.setPost(true, 1, &c);
    int ret = source.Open(0);
    ASSERT_GE(ret, 0);
    ret = source.Seek(0, SEEK_SIZE);
    AF_LOGD("size is %d\n", ret);
}

#include "ipList.h"

TEST(https, ipList)
{
    int size = sizeof(IpList) / sizeof(IpList[0]);

    if (size == 0) {
        return;
    }

    for (int i = 0; i < size; ++i) {
        globalSettings::getSetting().addResolve(ipListHots, IpList[i]);

        if (i > 0) {
            globalSettings::getSetting().removeResolve(ipListHots, IpList[i - 1]);
        }

        unique_ptr<IDataSource> source = unique_ptr<IDataSource>(dataSourcePrototype::create(ipListUrl));
        int ret = source->Open(0);
        ASSERT_GE(ret, 0);
        ASSERT_EQ(source->Seek(0, SEEK_SIZE), ipListFileSize);
    }
}

void UrlDataSourceCheckData(const string &url, int64_t fileSize)
{
    unique_ptr<IDataSource> source = static_cast<unique_ptr<IDataSource>>(dataSourcePrototype::create(url));
    int ret = source->Open(0);
    ASSERT_GE(ret, 0);
    int64_t size = source->Seek(0, SEEK_SIZE);
    ASSERT_EQ(size, fileSize);
    uint8_t c;
    int count = 100;
    for (int i = 0; i < count; ++i) {
        int64_t pos = llabs(random()) % (size);
        ret = source->Seek(pos, SEEK_SET);
        ASSERT_EQ(ret, pos);
        ret = source->Read(&c, 1);
        if (pos < size) {
            ASSERT_EQ(c, pos % 256);
        } else {
            ASSERT_EQ(ret, 0);// eos
        }
    }
}

int UrlDataSourceCheckDataThead()
{
    string url = "https://alivc-demo-vod.aliyuncs.com/sv/2e731281-17b5885d5af/2e731281-17b5885d5af.mp4";
    int64_t fileSize = 268435456;
    UrlDataSourceCheckData(url, fileSize);
    return -1;
}

TEST(UrlDataSource, check)
{
    setProperty("protected.network.http.http2", "ON");
    afThread *threads[3];
    for (auto &thread : threads) {
        thread = new afThread(UrlDataSourceCheckDataThead);
        thread->start();
    }
    for (auto &thread : threads) {
        thread->stop();
    }
    for (auto &thread : threads) {
        delete thread;
    }
}
