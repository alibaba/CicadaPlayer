//
// Created by lifujun on 2019/3/1.
//

#ifndef SOURCE_FILECNTL_H
#define SOURCE_FILECNTL_H

#include "utils/CicadaType.h"
#include <fcntl.h>
#include <string>

using namespace std;

class CICADA_CPLUS_EXTERN FileCntl {
public:
    explicit FileCntl(string filePath);

    ~FileCntl();
    int openFile(int flags);
    int openFile()
    {
        return openFile(O_RDWR | O_CREAT);
    }
    int openFileForRead()
    {
        return openFile(O_RDONLY);
    }
    int openFileForWrite()
    {
        return openFile(O_WRONLY | O_CREAT);
    }

    void openFileForOverWrite();

    void truncateFile(int length);

    int writeFile(uint8_t *buf, int size);
    int readFile(uint8_t *buf, int size);
    int64_t seekFile(int64_t offset, int whence);

    void closeFile();
    bool isValid();

private:
    string mFilePath;
    int mFd = -1;

};


#endif //SOURCE_FILECNTL_H
