//
// Created by lifujun on 2019/3/1.
//

#include "FileCntl.h"
#ifdef _WIN32
#include <basetsd.h>
#include <io.h>
#include <process.h>
typedef SSIZE_T ssize_t;
#else
#include <unistd.h>
#endif

#include <cassert>
#include <cerrno>

FileCntl::FileCntl(string filePath)
{
    mFilePath = std::move(filePath);
}

FileCntl::~FileCntl()
{
    if (mFd >= 0) {
        closeFile();
    }
}

int FileCntl::openFile(int flags)
{
#ifdef WIN32// Windows
#ifdef _MSC_VER
    _set_fmode(_O_BINARY);
#else //MinGW
    _fmode = _O_BINARY;
#endif// _MSC_VER
#endif// WIN32
    mFd = open(mFilePath.c_str(), flags, 0666);
    if (mFd > 0) {
        return 0;
    }
    return -errno;
}

void FileCntl::openFileForOverWrite()
{
    openFile(O_WRONLY | O_CREAT | O_TRUNC);
}

void FileCntl::truncateFile(int length)
{
#ifdef _WIN32
    _chsize_s(mFd, length);
#else
    ftruncate(mFd, length);
#endif
}


int64_t FileCntl::seekFile(int64_t offset, int whence)
{
    return lseek(mFd, offset, whence);
}

int FileCntl::writeFile(uint8_t *buf, int size)
{
    int writeSize = 0;
    do {
        ssize_t ret = write(mFd, buf + writeSize, size - writeSize);
        if (ret < 0) {
            return -errno;
        }
        assert(ret != 0);
        writeSize += (int) ret;
    } while (writeSize < size);
    return writeSize;
}

int FileCntl::readFile(uint8_t *buf, int size)
{
    return read(mFd, buf, size);
}

void FileCntl::closeFile()
{
    if (mFd >= 0) {
        close(mFd);
        mFd = -1;
    }
}

bool FileCntl::isValid()
{
    return mFd >= 0;
}


