//
// Created by moqi on 2019/11/15.
//

#include "gtest/gtest.h"
#include <demuxer/demuxerPrototype.h>
#include <data_source/dataSourcePrototype.h>
#include <demuxer/demuxer_service.h>
#include <utils/AFUtils.h>
#include "demuxerUtils.h"

using namespace Cicada;

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ignore_signal(SIGPIPE);
    return RUN_ALL_TESTS();
}

TEST(format, mp4)
{
    std::string url = "http://player.alicdn.com/video/aliyunmedia.mp4";
    auto source = dataSourcePrototype::create(url);
    source->Open(0);
    unique_ptr<demuxer_service> service = unique_ptr<demuxer_service>(new demuxer_service(source));
    int ret = service->initOpen();
    ASSERT_GE(ret, 0);
    service->close();
    delete source;
}

inline uint64_t getSize(const uint8_t *data, unsigned int len, unsigned int shift)
{
    uint64_t size(0);
    const uint8_t *dataE(data + len);

    for (data; data < dataE; ++data) {
        size = size << shift | *data;
    }

    return size;
};

TEST(format, aac)
{
    const char *hls_id3 = "id3v2_priv.com.apple.streaming.transportStreamTimestamp";
    std::string url = "https://devstreaming-cdn.apple.com/videos/streaming/examples/img_bipbop_adv_example_ts/a1/fileSequence0.aac";
    auto source = dataSourcePrototype::create(url);
    source->Open(0);
    unique_ptr<demuxer_service> service = unique_ptr<demuxer_service>(new demuxer_service(source));
    int ret = service->initOpen();
    ASSERT_GE(ret, 0);
    Source_meta *meta = nullptr;
    service->GetSourceMeta(&meta);
    Source_meta *meta1 = meta;

    while (meta1 != nullptr) {
        if (meta1->key && meta1->value) {
            AF_LOGD("%s:[%s]", meta1->key, meta1->value);
            int ptr = 0;

            if (strcmp(meta1->key, hls_id3) == 0) {
                uint8_t buf[8];
                int v;

                for (unsigned char &i : buf) {
                    if (sscanf(meta1->value + ptr, "\\x%02x", &v) == 1) {
                        ptr += 4;
                        i = v;
                    } else {
                        i = *(meta1->value + ptr);
                        ptr++;
                    }
                }

                uint64_t ps = getSize(buf, 8, 8);
                AF_LOGD("ps is %u\n", ps);
            }
        }

        meta1 = meta1->next;
    }

    releaseSourceMeta(meta);
    service->close();
    delete source;
}

int callback_read(void *arg, uint8_t *buffer, int size)
{
    Cicada::IDataSource *source = (Cicada::IDataSource *) arg;
    return source->Read(buffer, size);
}

int64_t callback_seek(void *arg, int64_t offset, int whence)
{
    Cicada::IDataSource *source = (Cicada::IDataSource *) arg;
    return source->Seek(offset, whence);
}

TEST(ts, pts)
{
    std::string url =
        "https://alivc-demo-vod.aliyuncs.com/ddb0c76ce153450081cd4c45118371a7/d30995ad97bc4643bf0a8c4cedd0c81f-007b1abb398f0e4c6f46d30b0125da41-sd-00001.ts";
    auto source = dataSourcePrototype::create(url);
    source->Open(0);
    int ret = 0;
    unique_ptr<demuxer_service> service = unique_ptr<demuxer_service>(new demuxer_service(nullptr));
    service->SetDataCallBack(callback_read, source, callback_seek, source, url.c_str());
    service->initOpen();
    ret = service->GetNbStreams();

    for (int i = 0; i < ret; i++) {
        service->OpenStream(i);
    }

    do {
        std::unique_ptr<IAFPacket> packet{};
        ret = service->readPacket(packet);

        if (packet) {
            AF_LOGD("pkt pts is %lld\n", packet->getInfo().pts);
            //          ASSERT_TRUE(packet->getInfo().pts != INT64_MIN);
        }
    } while (ret != 0);

    service->close();
    delete source;
}

TEST(first_seek, mp4)
{
    std::string url = "http://player.alicdn.com/video/aliyunmedia.mp4";
    testFirstSeek(url, 10000000, 1000000);
}

TEST(first_seek, hls)
{
    std::string url =
        "https://alivc-demo-vod.aliyuncs.com/ddb0c76ce153450081cd4c45118371a7/d30995ad97bc4643bf0a8c4cedd0c81f-e16b4635a4cb03424234c3a3d0e7f7e1-sd.m3u8";
    testFirstSeek(url, 100000000, 10000000);
}

TEST(mergeHeader, mp4)
{
    std::string url = "http://player.alicdn.com/video/aliyunmedia.mp4";
    test_mergeHeader(url, true);
    test_mergeHeader(url, false);
}

TEST(mergeHeader, ts)
{
    std::string url =
        "https://alivc-demo-vod.aliyuncs.com/ddb0c76ce153450081cd4c45118371a7/d30995ad97bc4643bf0a8c4cedd0c81f-007b1abb398f0e4c6f46d30b0125da41-sd-00001.ts";
    test_mergeHeader(url, true);
    test_mergeHeader(url, false);
}

TEST(mergeHeader, hls)
{
    std::string url =
        "https://alivc-demo-vod.aliyuncs.com/ddb0c76ce153450081cd4c45118371a7/d30995ad97bc4643bf0a8c4cedd0c81f-e16b4635a4cb03424234c3a3d0e7f7e1-sd.m3u8";
    test_mergeHeader(url, true);
    test_mergeHeader(url, false);
}

TEST(mergeHeader, hls_aes)
{
    std::string url =
        "https://alivc-demo-vod.aliyuncs.com/d2c89d7210d443109434685f45ed607b/45ed0cccd8092bf25ee33764b5a52be4-sd-encrypt-stream.m3u8";
    test_mergeHeader(url, true);
    test_mergeHeader(url, false);
}

TEST(mergeHeader, hls_multi)
{
    std::string url =
        "https://alivc-demo-vod.aliyuncs.com/59f748948daa4438b42e42db755ae01e/9d44b2b86d334c6b9df649e35ad0240f.m3u8";
    test_mergeHeader(url, true);
    test_mergeHeader(url, false);
}