//
// Created by lifujun on 2019/3/1.
//

#ifndef SOURCE_FILECNTL_H
#define SOURCE_FILECNTL_H

#include <string>
#include "utils/CicadaType.h"

using namespace std;

class CICADA_CPLUS_EXTERN FileCntl {
public:
    FileCntl(string filePath);

    ~FileCntl();

    void openFile();
    void openFileForRead();
    void openFileForWrite();

    int writeFile(uint8_t *buf, int size);
    int readFile(uint8_t *buf, int size);

    int64_t seekFile(int64_t offset, int whence);

    void closeFile();

private:
    void openFile(int flags);
    string mFilePath = "";
    int mFd = -1;

};


#endif //SOURCE_FILECNTL_H
