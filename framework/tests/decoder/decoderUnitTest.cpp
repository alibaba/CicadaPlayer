//
// Created by moqi on 2019/11/15.
//

#include <data_source/dataSourcePrototype.h>
#include <demuxer/demuxer_service.h>
#include <codec/decoderFactory.h>
#include <utils/timer.h>
#include "gtest/gtest.h"
#include <string>
#include <utils/AFUtils.h>

using namespace Cicada;
using namespace std;

static void test_codec(const string &url, AFCodecID codec, int flags)
{
    unique_ptr<IDecoder> decoder = decoderFactory::create(codec, flags, 0);
    ASSERT_TRUE(decoder);
    auto source = dataSourcePrototype::create(url);
    source->Open(0);
    auto *demuxer = new demuxer_service(source);
    int ret = demuxer->initOpen();
    Stream_meta smeta{};
    unique_ptr<streamMeta> meta = unique_ptr<streamMeta>(new streamMeta(&smeta));

    for (int i = 0; i < demuxer->GetNbStreams(); ++i) {
        demuxer->GetStreamMeta(meta, i, false);

        if (((Stream_meta *) (*meta))->codec == codec) {
            demuxer->OpenStream(i);
            ret = decoder->open(((Stream_meta *) (*meta)), nullptr, 0);
            ASSERT_TRUE(ret >= 0);
            break;
        }
    }

    std::unique_ptr<IAFPacket> packet{nullptr};

    do {
        int decoder_ret;
        ret = demuxer->readPacket(packet, 0);

        if (ret > 0) {
            decoder_ret = decoder->send_packet(packet, 0);
            ASSERT_TRUE(decoder_ret >= 0);
        }

        if (ret == -EAGAIN) {
            af_msleep(10);
            ret = 0;
            continue;
        }

        unique_ptr<IAFFrame> frame{nullptr};
        decoder_ret = decoder->getFrame(frame, 0);

        if (decoder_ret == -EAGAIN) {
            af_msleep(10);
            continue;
        }

        ASSERT_GE(decoder_ret, 0);

        if (frame) {
            frame->dump();
            break;
        }
    } while (ret >= 0);

    delete source;
    delete demuxer;
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ignore_signal(SIGPIPE);
    return RUN_ALL_TESTS();
}
#ifdef  __APPLE__
TEST(hardCodec, H264)
{
    std::string url = "http://player.alicdn.com/video/aliyunmedia.mp4";
    test_codec(url, AF_CODEC_ID_H264, DECFLAG_HW);
}
#endif

TEST(softCodec, H264)
{
    std::string url = "http://player.alicdn.com/video/aliyunmedia.mp4";
    test_codec(url, AF_CODEC_ID_H264, DECFLAG_SW);
}
TEST(softCodec, aac)
{
    std::string url = "http://player.alicdn.com/video/aliyunmedia.mp4";
    test_codec(url, AF_CODEC_ID_AAC, DECFLAG_SW);
}
