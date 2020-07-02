//
// Created by moqi on 2018/4/24.
//

#include <utils/af_string.h>
#include "dataSourceIO.h"

#define INITIAL_BUFFER_SIZE 32768
namespace Cicada {
    dataSourceIO::dataSourceIO(IDataSource *pDataSource) : mPDataSource(pDataSource)
    {
        init();
    }

    dataSourceIO::dataSourceIO(demuxer_callback_read read, demuxer_callback_seek seek, void *arg)
    {
        auto *read_buffer = static_cast<uint8_t *>(av_malloc(INITIAL_BUFFER_SIZE));
        mPb = avio_alloc_context(read_buffer, INITIAL_BUFFER_SIZE, 0, arg, read, nullptr, seek);
    }

    dataSourceIO::~dataSourceIO()
    {
        if (mPb) {
            av_freep(&mPb->buffer);
            av_freep(&mPb);
        }
    }

    int dataSourceIO::read_callback(void *arg, uint8_t *buffer, int size)
    {
        auto *pHandle = static_cast<dataSourceIO *>(arg);
        //     AF_LOGE("read_callback", "%s %d \n", __func__, size);
        int ret = pHandle->mPDataSource->Read(buffer, size);
        return ret ? ret : AVERROR_EOF;
    }

    int64_t dataSourceIO::seek_callback(void *arg, int64_t offset, int whence)
    {
        auto *pHandle = static_cast<dataSourceIO *>(arg);
        //      AF_LOGE("seek_callback", "%s %lld \n", __func__, offset);
        return pHandle->mPDataSource->Seek(offset, whence);
    }

    int dataSourceIO::init()
    {
        auto *read_buffer = static_cast<uint8_t *>(av_malloc(INITIAL_BUFFER_SIZE));
        mPb = avio_alloc_context(read_buffer, INITIAL_BUFFER_SIZE, 0, this, read_callback, nullptr, seek_callback);
        return 0;
    }

    static int cicada_get_line(AVIOContext *s, char *buf, int maxlen)
    {
        int i = 0;
        char c;

        do {
            c = (char) avio_r8(s);

            if (c && i < maxlen - 1) {
                buf[i++] = c;
            }
        } while (c != '\n' && c != '\r' && c);

        if (c == '\r' && avio_r8(s) != '\n' && !avio_feof(s)) {
            avio_skip(s, -1);
        }

        buf[i] = 0;
        return i;
    }

    int dataSourceIO::get_line(char *buf, int maxlen)
    {
        int len = cicada_get_line(mPb, buf, maxlen);

        while (len > 0 && AfString::isSpace(buf[len - 1])) {
            buf[--len] = '\0';
        }

        return len;
    }

    int64_t dataSourceIO::seek(int64_t offset, int whence)
    {
        return avio_seek(mPb, offset, whence);
    }

    bool dataSourceIO::isEOF()
    {
        return static_cast<bool>(avio_feof(mPb));
    }
}


