//
// Created by pingkai on 2019/12/31.
//
#define LOG_TAG "demuxerUtils"
#include "demuxerUtils.h"

#include <demuxer/demuxerPrototype.h>
#include <data_source/dataSourcePrototype.h>
#include <demuxer/demuxer_service.h>

#include "gtest/gtest.h"
using namespace Cicada;

void test_mergeHeader(std::string url, bool merge)
{
    auto source = dataSourcePrototype::create(url);
    source->Open(0);
    int ret = 0;
    unique_ptr<demuxer_service> service = unique_ptr<demuxer_service>(new demuxer_service(source));
    ret = service->createDemuxer(demuxer_type_unknown);
    service->getDemuxerHandle()->setBitStreamFormat(merge, merge);
    ret = service->initOpen();
    ret = service->GetNbStreams();
    int videoStreamId = -1;

    for (int i = 0; i < ret; i++) {
        std::unique_ptr<streamMeta> meta;
        service->GetStreamMeta(meta, i, false);

        if (((Stream_meta *) (*(meta.get())))->type == STREAM_TYPE_VIDEO
            || ((Stream_meta *) (*(meta.get())))->type == STREAM_TYPE_MIXED) {
            service->OpenStream(i);

            if (((Stream_meta *) (*(meta.get())))->type == STREAM_TYPE_MIXED) {
                unique_ptr<streamMeta> pMeta;
                Stream_meta *meta{};
                int nbSubStream = service->GetNbSubStream(i);

                for (int j = 0; j < nbSubStream; j++) {
                    service->GetStreamMeta(pMeta, GEN_STREAM_ID(i, j), true);
                    meta = (Stream_meta *) (pMeta.get());
                    AF_LOGD("get a stream %d\n", meta->type);

                    if (meta->type == STREAM_TYPE_VIDEO) {
                        videoStreamId = GEN_STREAM_ID(i, j);
                    }
                }
            } else {
                videoStreamId = i;
            }

            break;
        }
    }

    service->start();

    do {
        std::unique_ptr<IAFPacket> packet{};
        ret = service->readPacket(packet);

        if (packet) {
            if (videoStreamId < 0) {
                int i = GEN_STREAM_INDEX(packet->getInfo().streamIndex);
                unique_ptr<streamMeta> pMeta;
                Stream_meta *meta{};
                int nbSubStream = service->GetNbSubStream(i);

                for (int j = 0; j < nbSubStream; j++) {
                    service->GetStreamMeta(pMeta, GEN_STREAM_ID(i, j), true);
                    meta = (Stream_meta *) (pMeta.get());
                    AF_LOGD("get a stream %d\n", meta->type);

                    if (meta->type == STREAM_TYPE_VIDEO) {
                        videoStreamId = GEN_STREAM_ID(i, j);
                        // break;
                    }
                }
            }

            if (packet->getInfo().streamIndex == videoStreamId && packet->getInfo().flags) {
                if (merge) {
                    ASSERT_EQ(memcmp(packet->getData(), "\0\0\0\1", 4), 0);
                } else {
                    ASSERT_NE(memcmp(packet->getData(), "\0\0\0\1", 4), 0);
                }

                break;
            }
        }
        if (ret < 0)
            break;
    } while (ret != 0);

    service->close();
    delete source;
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