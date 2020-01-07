//
// Created by moqi on 2018/1/23.
//

#include "file_data_source.h"
#include <fcntl.h>
#include <unistd.h>
#include <log.h>
#include <cerrno>

#define TAG "fileDataSource"

namespace Cicada {
    fileDataSource::fileDataSource()
    {
        mFd = 0;
    }

    fileDataSource::~fileDataSource()
    {
        Close();
    }

    int fileDataSource::Open(const string &path)
    {
        mPath = path;

        if (mPath.c_str() == nullptr) {
            return -1;
        }

        mFd = ::open(mPath.c_str(), O_RDONLY);

        if (mFd < 0) {
            AF_LOGE(TAG, "open %s error %d(%s)\n", mPath.c_str(), errno, strerror(errno));
            return mFd;
        }

        return 0;
    }

    void fileDataSource::Close()
    {
        if (mFd > 0) {
            ::close(mFd);
            mFd = 0;
        }
    }

    int64_t fileDataSource::Seek(int64_t offset, int whence)
    {
        if (mFd <= 0) {
            return -1;
        }

        return ::lseek(mFd, offset, whence);
    }

    ssize_t fileDataSource::Read(void *buf, size_t nbyte)
    {
        return ::read(mFd, buf, nbyte);
    }
}
