#include <iostream>
#include "dataSourceIO.h"
#include "IDataSource.h"
#include "dataSourcePrototype.h"
#include <fcntl.h>
#include <utils/frame_work_log.h>
#include <pthread.h>
#include <utils/timer.h>
#include <utils/tty.h>
#include <../../plugin/BiDataSource.h>

using namespace Cicada;
typedef struct mainCont_t {
    IDataSource *pDownloader;
    bool isEOS;
    bool isCanceled;
    int outFd;
    int64_t fileSize;

} mainCont;

static void *read_thread(void *arg)
{
    mainCont *pHandle = (mainCont *) arg;
    auto *pDownloader = pHandle->pDownloader;
    int total_size = 0;
    int ret;
    int read_size = 1024;
    auto *buffer = static_cast<uint8_t *>(malloc(static_cast<size_t>(read_size + 1)));

    do {
        ret = static_cast<int>(pDownloader->Read(buffer, read_size));

        if (ret > 0) {
            total_size += ret;

            if (pHandle->outFd > 0) {
                write(pHandle->outFd, buffer, static_cast<size_t>(ret));
            }

            //    buffer[ret] = 0;
            //    printf("%s", buffer);
        }
    } while (ret > 0 && !pHandle->isCanceled);

    //   printf("\n");
    if (ret < 0) {
        printf("read error %d (%s)\n", ret, pDownloader->Get_error_info(ret).c_str());
    }

    printf("total_size is %d\n", total_size);
    free(buffer);
    pHandle->isEOS = true;
    return nullptr;
}

static void *seek_thread
(void *arg)
{
    mainCont *pHandle = (mainCont *) arg;
    auto *pDownloader = pHandle->pDownloader;
    int total_size = 0;
    int ret;
    int read_size = 1024;
    int64_t seekPos = 0;
    auto *buffer = static_cast<uint8_t *>(malloc(static_cast<size_t>(read_size + 1)));

    do {
        ret = pDownloader->Read(buffer, read_size);

        if (ret > 0) {
            total_size += ret;
            //    buffer[ret] = 0;
            //    printf("%s", buffer);
        }

//        seekPos = rand() % pHandle->fileSize;
//        printf("seek to %lli\n", seekPos);
//        pDownloader->Seek(seekPos, SEEK_SET);
    } while (ret > 0 && !pHandle->isCanceled);

    //   printf("\n");
    if (ret < 0) {
        printf("read error %d (%s)\n", ret, pDownloader->Get_error_info(ret).c_str());
    }

    printf("total_size is %d\n", total_size);
    free(buffer);
    pHandle->isEOS = true;
    return nullptr;
}


int main(int argc, const char **argv)
{
    log_enable_color(1);
//    if (argc < 3) {
//        printf("Usage: %s fileName outPutPath\n", argv[0]);
//        return -1;
//    }
    std::string url = BiDataSource::createUrl(
            "/Users/moqi/work/xx/apsaraPlayer/CicadaPlayer/platform/Apple/source/cmake-build-debug/mediaPlayer.out/framework.out/aliyunmedia.mp4",
            "http://player.alicdn.com/video/aliyunmedia.mp4", 146057805);
    int ret;
    mainCont *pHandle = static_cast<mainCont *>(malloc(sizeof(mainCont)));
    pHandle->pDownloader = dataSourcePrototype::create(url);
    pHandle->isEOS = false;
    IDataSource::SourceConfig config{};
    config.low_speed_time = 5;
    config.low_speed_limit = 1;
    config.connect_time = 5;
    pHandle->pDownloader->Set_config(config);
    config.connect_time = 0;
    pHandle->pDownloader->Get_config(config);
    AF_LOGD("config.connect_time is %d\n", config.connect_time);
    std::string location("location");
    ret = pHandle->pDownloader->Open(0);

    if (ret < 0) {
        AF_LOGE("open error!");
        goto fail;
    }

//    printf("location is %s\n", pHandle->pDownloader->Get_option(location).c_str());
//    pHandle->fileSize = pHandle->pDownloader->Seek(0, SEEK_SIZE);
//    printf("fileSize is %lld\n", pHandle->fileSize);
//    {
//        int ret = 0;
//        dataSourceIO *du = new dataSourceIO(pHandle->pDownloader);
//        char *buffer = static_cast<char *>(malloc(1024));
//        memset(buffer, 0, 1024);
//        while (du->get_line(buffer, 1024) > 0)
//            printf("get_line %s \n", buffer);
//        du->seek(0, SEEK_SET);
//        while (du->get_line(buffer, 1024) > 0)
//            printf("get_line %s \n", buffer);
//
//        delete du;
//        free(buffer);
//    }
//    return 0;
    argv[2] = "xx.flv";
    unlink(argv[2]);
    pHandle->outFd = open(argv[2], O_CREAT | O_WRONLY, 0666);

    if (pHandle->outFd < 0) {
        printf("open %s error \n", argv[2]);
        goto fail;
    }

    pthread_t read_thread_id;
    pthread_create(&read_thread_id, NULL, read_thread, pHandle);

    while (!pHandle->isEOS) {
        char ch = -1;
        tty_set_noblock();

        while (!pHandle->isEOS) {
            do {
                ch = static_cast<char>(getchar());

                if (ch == -1)
                    af_usleep(100000);
            } while ((ch == -1 || ch == '\n') && !pHandle->isEOS);

            switch (ch) {
                case 'q':
                    pHandle->isCanceled = true;
                    pHandle->pDownloader->Interrupt(true);
                    printf("Quiting\n");
                    break;

                default:
                    //printf("unknown command %c\n",ch);
                    break;
            }
        }

        tty_reset();
    }

    pthread_join(read_thread_id, NULL);
    close(pHandle->outFd);
    return 0;
    ret = pHandle->pDownloader->Open("https://www.baidu.com/img/bd_logo1.png");

    if (ret < 0) {
        std::cout << "open error!" << std::endl;
        goto fail;
    }

    pthread_create(&read_thread_id, NULL, read_thread, pHandle);

    while (!pHandle->isEOS) {
        char ch = -1;
        tty_set_noblock();

        while (!pHandle->isEOS) {
            do {
                ch = static_cast<char>(getchar());

                if (ch == -1)
                    af_usleep(100000);
            } while ((ch == -1 || ch == '\n') && !pHandle->isEOS);

            switch (ch) {
                case 'q':
                    pHandle->isCanceled = true;
                    pHandle->pDownloader->Interrupt(true);
                    printf("Quiting\n");
                    break;

                default:
                    //printf("unknown command %c\n",ch);
                    break;
            }
        }

        tty_reset();
    }

    pthread_join(read_thread_id, NULL);

    if (pHandle->outFd > 0)
        close(pHandle->outFd);

fail:
    pHandle->pDownloader->Close();
    delete pHandle->pDownloader;
    free(pHandle);
    return 0;
}