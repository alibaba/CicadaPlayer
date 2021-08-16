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

void FileCntl::openFile(int flags)
{
#ifdef WIN32// Windows
#ifdef _MSC_VER
    _set_fmode(_O_BINARY);
#else //MinGW
    _fmode = _O_BINARY;
#endif// _MSC_VER
#endif// WIN32
    mFd = open(mFilePath.c_str(), flags, 0666);
}

void FileCntl::openFile()
{
    openFile(O_RDWR | O_CREAT);
}

void FileCntl::openFileForRead()
{
    openFile(O_RDONLY);
}

void FileCntl::openFileForWrite()
{
    openFile(O_WRONLY | O_CREAT);
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


