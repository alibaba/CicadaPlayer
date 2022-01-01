//
// Created by pingkai on 2019/12/31.
//
#define LOG_TAG "demuxerUtils"
#include "demuxerUtils.h"
#include <data_source/dataSourcePrototype.h>
#include <demuxer/demuxerPrototype.h>
#include <demuxer/demuxer_service.h>
#include <utils/frame_work_log.h>

extern "C" {
#include <framework/utils/ffmpeg_utils.h>
#include <libavutil/intreadwrite.h>
};

#include "gtest/gtest.h"
using namespace Cicada;

void test_demuxUrl(const string &url, header_type merge, Stream_type type, const std::function<void(demuxer_service* demuxer, IAFPacket *pkt)> &judgeFunc)
{
    auto source = dataSourcePrototype::create(url);
    source->Open(0);
    int ret = 0;
    unique_ptr<demuxer_service> service = unique_ptr<demuxer_service>(new demuxer_service(source));
    ret = service->createDemuxer(demuxer_type_unknown);
    service->getDemuxerHandle()->setBitStreamFormat(merge, merge);
    ret = service->initOpen();
    ret = service->GetNbStreams();
    int streamId = -1;

    for (int i = 0; i < ret; i++) {
        std::unique_ptr<streamMeta> meta;
        service->GetStreamMeta(meta, i, false);

        if (((Stream_meta *) (*(meta.get())))->type == type || ((Stream_meta *) (*(meta.get())))->type == STREAM_TYPE_MIXED) {
            service->OpenStream(i);

            if (((Stream_meta *) (*(meta.get())))->type == STREAM_TYPE_MIXED) {
                unique_ptr<streamMeta> pMeta;
                Stream_meta *meta{};
                int nbSubStream = service->GetNbSubStream(i);

                for (int j = 0; j < nbSubStream; j++) {
                    service->GetStreamMeta(pMeta, GEN_STREAM_ID(i, j), true);
                    meta = (Stream_meta *) (pMeta.get());
                    AF_LOGD("get a stream %d\n", meta->type);

                    if (meta->type == type) {
                        streamId = GEN_STREAM_ID(i, j);
                    }
                }
            } else {
                streamId = i;
            }

            break;
        }
    }

    service->start();

    do {
        std::unique_ptr<IAFPacket> packet{};
        ret = service->readPacket(packet);

        if (packet) {
            if (streamId < 0) {
                int i = GEN_STREAM_INDEX(packet->getInfo().streamIndex);
                unique_ptr<streamMeta> pMeta;
                Stream_meta *meta{};
                int nbSubStream = service->GetNbSubStream(i);

                for (int j = 0; j < nbSubStream; j++) {
                    service->GetStreamMeta(pMeta, GEN_STREAM_ID(i, j), true);
                    meta = (Stream_meta *) (pMeta.get());
                    AF_LOGD("get a stream %d\n", meta->type);

                    if (meta->type == type) {
                        streamId = GEN_STREAM_ID(i, j);
                        // break;
                    }
                }
            }

            if (packet->getInfo().streamIndex == streamId && judgeFunc != nullptr) {
                judgeFunc(service.get(), packet.get());
                break;
            }
        }

    } while (ret >= 0 || ret == -EAGAIN);

    service->close();
    delete source;
}


void test_mergeHeader(std::string url, header_type merge)
{
    test_demuxUrl(url, merge, STREAM_TYPE_VIDEO, [merge](demuxer_service* demuxer,IAFPacket *packet) -> void {
        if (merge == header_type::header_type_merge) {
            ASSERT_EQ(memcmp(packet->getData(), "\0\0\0\1", 4), 0);
        } else if(merge == header_type::header_type_extract){
            ASSERT_NE(memcmp(packet->getData(), "\0\0\0\1", 4), 0);
        }
    });
}

void test_mergeAudioHeader(const std::string &url, header_type merge)
{
    test_demuxUrl(url, merge, STREAM_TYPE_AUDIO, [merge](demuxer_service* demuxer,IAFPacket *packet) -> void {
        uint8_t *data = packet->getData();
        if (merge == header_type::header_type_merge) {
            ASSERT_EQ(AV_RB16(data) & 0xfff0, 0xfff0);
            int mFrameLength =
                    (data[3] & 0x03) << 11 | (data[4] & 0xFF) << 3 | (data[5] & 0xE0) >> 5;
            ASSERT_EQ(packet->getSize() ,mFrameLength);
        } else if(merge == header_type::header_type_extract){
            ASSERT_NE(AV_RB16(data) & 0xfff0, 0xfff0);
        }
    });
}

void testFirstSeek(const string &url, int64_t time, int64_t abs_error)
{
    auto source = dataSourcePrototype::create(url);
    source->Open(0);
    int ret = 0;
    unique_ptr<demuxer_service> service = unique_ptr<demuxer_service>(new demuxer_service(source));
    service->Seek(time, 0, -1);
    ret = service->initOpen();
    ret = service->GetNbStreams();

    for (int i = 0; i < ret; i++) {
        service->OpenStream(i);
    }

    service->start();

    do {
        std::unique_ptr<IAFPacket> packet{};
        ret = service->readPacket(packet);

        if (packet) {
            AF_LOGD("pkt timePosition is %lld\n", packet->getInfo().timePosition);
            ASSERT_NEAR(packet->getInfo().timePosition, time, abs_error);
            break;
        }
    } while (ret != 0);

    service->close();
    delete source;
}


void test_encryptionInfo(const std::string &url ,  Stream_type type ,header_type merge)
{
    test_demuxUrl(url, merge, STREAM_TYPE_VIDEO, [](demuxer_service* demuxer,IAFPacket *pkt) -> void {
        IAFPacket::EncryptionInfo info{};
        bool ret = pkt->getEncryptionInfo(&info);
        ASSERT_EQ(ret, true);
        ASSERT_EQ(info.subsamples.size(), 1);
        ASSERT_EQ(pkt->getSize() , info.subsamples.front().bytes_of_clear_data + info.subsamples.front().bytes_of_protected_data);
    });
}

void test_metaKeyInfo(const std::string& url, Stream_type type ) {
    test_demuxUrl(url, header_type_no_touch, type, [](demuxer_service *demuxer, IAFPacket *pkt) -> void {
        int index = pkt->getInfo().streamIndex;
        unique_ptr<streamMeta> mCurrentStreamMeta{};
        demuxer->GetStreamMeta(mCurrentStreamMeta, index, false);
        auto *meta = (Stream_meta *) (mCurrentStreamMeta.get());
        ASSERT_NE(meta->keyUrl, nullptr);
        ASSERT_NE(meta->keyFormat, nullptr);
    });
}

void test_csd( const std::string& url , header_type merge) {
    test_demuxUrl(url, header_type_no_touch, STREAM_TYPE_VIDEO, [merge](demuxer_service *demuxer, IAFPacket *pkt) -> void {
        int index = pkt->getInfo().streamIndex;
        unique_ptr<streamMeta> mCurrentStreamMeta{};
        demuxer->GetStreamMeta(mCurrentStreamMeta, index, false);
        auto *meta = (Stream_meta *) (mCurrentStreamMeta.get());

        if (meta->codec == AF_CODEC_ID_H264) {

            uint8_t *sps_data = nullptr;
            uint8_t *pps_data = nullptr;
            int sps_data_size = 0;
            int pps_data_size = 0;
            int naluLengthSize = 0;

            int ret = parse_h264_extraData(CodecID2AVCodecID(AF_CODEC_ID_H264),meta->extradata, meta->extradata_size,
                                           &sps_data,&sps_data_size,
                                           &pps_data,&pps_data_size,
                                           &naluLengthSize);
            ASSERT_GE(ret, 0);
            ASSERT_NE(sps_data, nullptr);
            ASSERT_NE(pps_data, nullptr);

            free(sps_data);
            free(pps_data);
        }else if(meta->codec == AF_CODEC_ID_HEVC){

            uint8_t *vps_data = nullptr;
            uint8_t *sps_data = nullptr;
            uint8_t *pps_data = nullptr;
            int vps_data_size = 0;
            int sps_data_size = 0;
            int pps_data_size = 0;
            int naluLengthSize = 0;

            int ret = parse_h265_extraData(CodecID2AVCodecID(AF_CODEC_ID_HEVC),meta->extradata, meta->extradata_size,
                                           &vps_data,&vps_data_size,
                                           &sps_data,&sps_data_size,
                                           &pps_data,&pps_data_size,
                                           &naluLengthSize);
            ASSERT_GE(ret, 0);
            ASSERT_NE(vps_data, nullptr);
            ASSERT_NE(sps_data, nullptr);
            ASSERT_NE(pps_data, nullptr);

            free(sps_data);
            free(pps_data);
            free(vps_data);
        }
    });
}
