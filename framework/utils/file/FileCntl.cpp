//
// Created by lifujun on 2019/3/1.
//

#include "FileCntl.h"
#ifdef _WIN32
    #include <io.h>
    #include <process.h>
#else
    #include <unistd.h>
#endif
#include <fcntl.h>

FileCntl::FileCntl(string filePath)
{
    mFilePath = filePath;
}

FileCntl::~FileCntl()
{
    if (mFd > 0) {
        closeFile();
    }
}

void FileCntl::openFile()
{
#ifdef WIN32
    _set_fmode(_O_BINARY);
#endif // WIN32
    mFd = open(mFilePath.c_str(), O_RDWR | O_CREAT, 0666);
}

int64_t FileCntl::seekFile(int64_t offset, int whence)
{
    return lseek(mFd, offset, whence);
}

int FileCntl::writeFile(uint8_t *buf, int size)
{
    int writeSize = write(mFd, buf, size);
    return writeSize;
}

void FileCntl::closeFile()
{
    if (mFd >= 0) {
        close(mFd);
        mFd = -1;
    }
}


