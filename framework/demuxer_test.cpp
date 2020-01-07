//
// Created by moqi on 2018/2/1.
//

#define LOG_TAG "DemuxerTest"

#include <utils/tty.h>
#include <utils/frame_work_log.h>
#include <utils/errors/framework_error.h>
#include <pthread.h>
#include <cerrno>
#include <utils/timer.h>
#include "demuxer/demuxer_service.h"

#include "data_source/IDataSource.h"
#include "codec/IDecoder.h"
#include "codec/videoDecoderFactory.h"
#include <data_source/dataSourcePrototype.h>
#include <memory>

using namespace Cicada;
using namespace std;

typedef struct mainCont_t {
    IDataSource *pDataSource;
    bool isEOS;
    bool isCanceled;
    bool isDataSourceOpened;

    demuxer_service *pDemuxer;

    int mVideoIndex = -1;
    int mAudioIndex = -1;
    int mSubtitleIndex = -1;
    enum AFCodecID mVCodec = AF_CODEC_ID_NONE;

} mainCont;

static int read_callback(void *arg, uint8_t *buffer, int size)
{
    mainCont *pHanle = static_cast<mainCont *>(arg);
    return pHanle->pDataSource->Read(buffer, size);
}

static int64_t seek_callback(void *arg, int64_t offset, int whence)
{
    mainCont *pHanle = static_cast<mainCont *>(arg);
    AF_LOGE("%s %lld \n", __func__, offset);
    return pHanle->pDataSource->Seek(offset, whence);
}

static void *demuxer_thread(void *arg)
{
    mainCont *pHandle = static_cast<mainCont *>(arg);
    int ret = 1;
    IDecoder *video_decoder = nullptr;

    if (pHandle->mVideoIndex >= 0) {
        Stream_meta meta;
        pHandle->pDemuxer->GetStreamMeta(&meta, pHandle->mVideoIndex);
        void *par = pHandle->pDemuxer->getCodecPar(pHandle->mVideoIndex);
        video_decoder = VideoDecoderFactoryManager::create(meta.codec, 0, true);
        ret = video_decoder->open(&meta, nullptr, DECFLAG_SW);

        if (ret < 0) {
            AF_LOGE("open video decoder error\n");
        }

        releaseMeta(&meta);
    }

    //   int fd = open("./xx", O_CREAT | O_WRONLY, 0666);
    pHandle->pDemuxer->start();
    std::unique_ptr<IAFPacket> pPacket;
    int index = 0;

    while (!pHandle->isCanceled) {
        if (pPacket == nullptr) {
            ret = pHandle->pDemuxer->readPacket(pPacket);

            if (ret == -EAGAIN) {
                AF_LOGD("read packet time out\n");
                af_msleep(10);
                continue;
            }
        }

        if (pPacket) {
            AF_LOGD("pFrame->pts is %lld index is %d size is %d\n", pPacket->getInfo().pts, pPacket->getInfo().streamIndex,
                    pPacket->getSize());

//                if (pFrame->streamIndex == 0){
//                    write(fd,pFrame->pBuffer,pFrame->size);
//                }
            if (pPacket->getInfo().streamIndex == pHandle->mVideoIndex && video_decoder) {
                ret = video_decoder->send_packet(pPacket, 0);
                AF_TRACE;

                if (ret & STATUS_GOT_PIC) {
                    unique_ptr<IAFFrame> frame;
                    ret = video_decoder->getFrame(frame, 0);
                    AF_LOGD("decode a video picture %lld\n", frame->getInfo().pts);
                    index++;
                }

                if (!(ret & STATUS_RETRY_IN)) {
                    AF_TRACE;
                } else {
                    AF_TRACE;
                    af_msleep(10);
                }
            } else if (pPacket->getInfo().streamIndex == pHandle->mSubtitleIndex) {
                //  pPacket->getInfo().dump();
                AF_LOGI("%s\n", pPacket->getData());
                pPacket = nullptr;
                af_msleep(1000);
            } else {
                pPacket = nullptr;
            }

            index++;
        }

        if (ret == 0) {
            AF_LOGD("demuxer eos\n");
            pHandle->pDemuxer->Seek(0, 0, -1);
            //  break;
        }
    }

    pHandle->isEOS = true;
    //  close(fd);
    delete video_decoder;
    video_decoder = nullptr;
    pHandle->pDemuxer->stop();
    return nullptr;
}

int main(int argc, const char **argv)
{
    log_enable_color(1);
    mainCont *pHandle = static_cast<mainCont *>(malloc(sizeof(mainCont)));
    memset(pHandle, 0, sizeof(mainCont));
    pHandle->mVideoIndex = pHandle->mAudioIndex = pHandle->mSubtitleIndex = -1;
    int ret;
    const char *url;

    if (argc > 1) {
        url = argv[1];
    } else {
        url = url = "http://player.alicdn.com/video/aliyunmedia.mp4";
    }

    pHandle->pDataSource = dataSourcePrototype::create(url);
    IDataSource::SourceConfig config{};
    config.low_speed_time = 5;
    config.low_speed_limit = 1;
    pHandle->pDataSource->Set_config(config);
    bool noFile = false;
    // use avformat to demux webvtt
    ret = pHandle->pDataSource->Open(0);

    if (ret < 0) {
        AF_LOGE("can't open url %s\n", url);
        AF_LOGE("ret is %04x %s", -ret, framework_err2_string(ret));

        if (ret != FRAMEWORK_ERR_PROTOCOL_NOT_SUPPORT) {
            delete pHandle->pDataSource;
            free(pHandle);
            return -1;
        } else {
            noFile = true;
        }
    }

    pHandle->isDataSourceOpened = true;
    int64_t fileSize = pHandle->pDataSource->Seek(0, SEEK_SIZE);
    printf("fileSize is %lld\n", fileSize);
    auto *pDemuxer = new demuxer_service(pHandle->pDataSource);
    pDemuxer->setNoFile(noFile);
    pHandle->pDemuxer = pDemuxer;
    pDemuxer->initOpen(noFile ? demuxer_type_bit_stream : demuxer_type_unknown);

    if (ret >= 0) {
        int nbStream = pDemuxer->GetNbStreams();
        AF_LOGI("file have %d streams\n", nbStream);
        unique_ptr<streamMeta> smeta;

        for (int i = 0; i < nbStream; ++i) {
            pDemuxer->GetStreamMeta(smeta, i);
            Stream_meta *meta = (Stream_meta *) (*(smeta.get()));
            AF_LOGI("duration is %lld\n", meta->duration);

            if (meta->type == STREAM_TYPE_VIDEO && pHandle->mVideoIndex < 0) {
                AF_LOGD("get a video stream %d\n", i);
                pDemuxer->OpenStream(i);
                AF_LOGD(" %d x %d\n", meta->width, meta->height);
                AF_LOGD(" bandwidth is %llu\n", meta->bandwidth);
                pHandle->mVideoIndex = i;
                pHandle->mVCodec = meta->codec;
            } else if (meta->type == STREAM_TYPE_AUDIO && pHandle->mAudioIndex < 0) {
                AF_LOGE("get a audio stream %d \n", i);
                pDemuxer->OpenStream(i);
                AF_LOGD("lang is %s\n", meta->lang);
                pHandle->mAudioIndex = i;
            } else if (meta->type == STREAM_TYPE_SUB && pHandle->mSubtitleIndex < 0) {
                AF_LOGD("get a subtitle stream %d\n", i);
                AF_LOGD("lang is %s\n", meta->lang);
                pDemuxer->OpenStream(i);
                pHandle->mSubtitleIndex = i;
            }

            if (pHandle->mAudioIndex >= 0 && pHandle->mVideoIndex >= 0 && pHandle->mSubtitleIndex >= 0) {
                break;
            }
        }
    } else {
        AF_LOGE("ret is %04x %s", -ret, framework_err2_string(ret));
        return 0;
    }

    pthread_t demuxer_thread_id;
    pthread_create(&demuxer_thread_id, nullptr, demuxer_thread, pHandle);

    while (!pHandle->isEOS && !pHandle->isCanceled) {
        char ch = -1;
        tty_set_noblock();

        do {
            ch = static_cast<char>(getchar());

            if (ch == -1) {
                af_usleep(100000);
            }
        } while ((ch == -1 || ch == '\n') && !pHandle->isEOS);

        switch (ch) {
            case 'q':
                pHandle->isCanceled = true;
                pHandle->pDataSource->Interrupt(true);
                printf("Quiting\n");
                break;

            default:
                //printf("unknown command %c\n",ch);
                break;
        }

        tty_reset();
    }

    pthread_join(demuxer_thread_id, nullptr);
    pDemuxer->close();
    delete pDemuxer;
    delete pHandle->pDataSource;
    free(pHandle);
    return 0;
}

