//
// Created by moqi on 2018-12-18.
//

#include <utils/frame_work_log.h>
#include <fcntl.h>
#include "data_source/cachedSource.h"
#include "ISliceManager.h"
#include <unistd.h>
#include <utils/property.h>

using namespace Cicada;
#define SLICE_SIZE 1024*32

int main()
{
    setProperty("ro.SliceManager.capacityM", "20"); //M
    setProperty("ro.SliceManager.sliceSizeK", "32"); //K
    auto *source = new cachedSource("http://player.alicdn.com/video/aliyunmedia.mp4", 10000);
    auto *pSliceManager = ISliceManager::getManager();
    source->setSliceManager(pSliceManager);
    SourceConfig config{};
    config.low_speed_time = 5;
    config.low_speed_limit = 1;
    source->getDataSource()->Set_config(config);
    int ret = source->Open(0);

    if (ret < 0) {
        AF_LOGE("source open error\n");
        return ret;
    }

    auto *sd = new cachedSource::describer(*source);
    void *buffer = malloc(SLICE_SIZE);

    do {
        ret = sd->read(buffer, SLICE_SIZE);
        AF_LOGD("read size is %d\n", ret);
    } while (ret > 0);

    sd->seek(0, SEEK_SET);
    unlink("9e09053de13f4fffdf83fea670a854cb.mp4");
    int fd = ::open("9e09053de13f4fffdf83fea670a854cb.mp4", O_CREAT | O_WRONLY, 0666);

    do {
        ret = sd->read(buffer, SLICE_SIZE);
        AF_LOGD("read size is %d\n", ret);

        if (ret > 0) {
            ::write(fd, buffer, ret);
        }
    } while (ret > 0);

    ::close(fd);
    free(buffer);
    source->Close();
    delete source;
    delete pSliceManager;
    return 0;
}

