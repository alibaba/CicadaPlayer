//
// Created by moqi on 2018-12-17.
//
#include <utils/frame_work_log.h>
#include <fcntl.h>
#include "slice.h"
#include "data_source/DataSourceFactory.h"
#include <unistd.h>

using namespace Cicada;

void testSlice()
{
    uint8_t buffer[64];

    for (int i = 0; i < 63; ++i) {
        buffer[i] = static_cast<uint8_t>(0x30 + i);
    }

    buffer[63] = 0;
    AF_LOGD("buffer is %s\n", buffer);
    auto slice1 = new slice(32, 0);
    slice1->write(buffer, 32);
    auto slice2 = new slice(32, 32);
    slice2->write(buffer + 32, 32);
    slice1->readAt(buffer, 32, 0);
    slice2->readAt(buffer + 32, 32, 0);
    AF_LOGD("buffer is %s\n", buffer);
}

#define SLICE_SIZE  1024 *16

int readSlice(IDataSource *source, const uint8_t *ReadBuffer)
{
    int sizeRead = 0;
    int ret;

    while (sizeRead < SLICE_SIZE) {
        ret = source->Read((void *) (ReadBuffer + sizeRead), SLICE_SIZE - sizeRead);

        if (ret <= 0) {
            break;
        } else {
            sizeRead += ret;
        }
    }

    return sizeRead;
}

void testSliceBuffer()
{
    IDataSource *source = DataSourceFactory::create("http://player.alicdn.com/video/aliyunmedia.mp4");
    int ret = source->Open(0);

    if (ret < 0) {
        AF_LOGE("source open error\n");
        return;
    }

    auto fileSize = static_cast<uint64_t>(source->Seek(0, SEEK_SIZE));
    sliceBuffer *buffer = new sliceBuffer(SLICE_SIZE, fileSize, nullptr);
    AF_LOGD("fileSize is %d\n", fileSize);
    uint8_t *ReadBuffer = new uint8_t[SLICE_SIZE];
    int writeSize = 0;

    while (true) {
        ret = readSlice(source, ReadBuffer);

        //   AF_LOGD("read %d from source\n",ret);
        if (ret <= 0) {
            break;
        }

        buffer->writeAt(ReadBuffer, ret, writeSize);
        writeSize += ret;
    }

    AF_LOGD("writeSize is %d\n", writeSize);
    source->Close();
    delete source;
    int fd = ::open("slice.mp4", O_CREAT | O_WRONLY, 0666);
    int readSize = 0;

    do {
        ret = buffer->readAt(ReadBuffer, SLICE_SIZE, readSize);

        if (ret > 0) {
            ::write(fd, ReadBuffer, ret);
            readSize += ret;
        }

        AF_LOGD("ret is %d\n", ret);
    } while (ret > 0);

    AF_LOGD("readSize is %d\n", readSize);
    ::close(fd);
    delete[] ReadBuffer;
    buffer->dump();
    delete buffer;
}

int main()
{
    testSlice();
    testSliceBuffer();
}

