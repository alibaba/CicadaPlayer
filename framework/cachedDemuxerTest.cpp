//
// Created by moqi on 2018-12-18.
//

#define LOG_TAG "cachedDemuxer"
#include <utils/frame_work_log.h>
#include "data_source/cachedSource.h"
#include "demuxer/demuxer_service.h"

using namespace alivc;

typedef struct mainCont_t {
    cachedSource *pSource;
    cachedSource::describer *pd;
} mainCont;

int cachedSource_callback_read(void *arg, uint8_t *buffer, int size)
{
    mainCont *pHandle = static_cast<mainCont *>(arg);
    return pHandle->pd->read(buffer, size);

}

int64_t cachedSource_callback_seek(void *arg, int64_t offset, int whence)
{
    mainCont *pHandle = static_cast<mainCont *>(arg);
    return pHandle->pd->seek(offset, whence);

}

int main(int argc, const char **argv)
{
    mainCont *pHandle = static_cast<mainCont *>(malloc(sizeof(mainCont)));
    memset(pHandle, 0, sizeof(mainCont));
    const char *url;
    if (argc > 1)
        url = argv[1];
    else
        url = "http://player.alicdn.com/video/aliyunmedia.mp4";
    pHandle->pSource = new cachedSource(url);
    SourceConfig config{};
    config.low_speed_time = 5;
    config.low_speed_limit = 1;
    pHandle->pSource->getDataSource()->Set_config(config);
    int ret = pHandle->pSource->Open(0);
    if (ret < 0) {
        AF_LOGE("source open error\n");
        return ret;
    }
    pHandle->pd = new cachedSource::describer(*(pHandle->pSource));
    auto *pDemuxer = new demuxer_service(nullptr);
    pDemuxer->SetDataCallBack(cachedSource_callback_read, pHandle, cachedSource_callback_seek, pHandle, nullptr);
    ret = pDemuxer->initOpen(demuxer_type_bit_stream);
    if (ret >= 0) {
        int nbStream = pDemuxer->GetNbStreams();
        AF_LOGI("file have %d streams\n", nbStream);
        // open all stream in demuxer
        Stream_meta meta{};
        for (int i = 0; i < nbStream; ++i) {
            pDemuxer->GetStreamMeta(&meta, i);
            releaseMeta(&meta);
        }
    } else {
        AF_LOGE("open demuxer error %d\n", ret);
    }
    pDemuxer->close();
    pHandle->pSource->Close();
    delete pHandle->pd;
    delete pHandle->pSource;
    delete pDemuxer;
    free(pHandle);
}

