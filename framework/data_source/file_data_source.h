//
// Created by moqi on 2018/1/23.
//

#ifndef FRAMEWORK_FILEDATASOURCE_H
#define FRAMEWORK_FILEDATASOURCE_H

#include "IDataSource.h"

namespace Cicada{

    class fileDataSource : public IDataSource {
    public:
        bool probe(string &path)
        {
            return true;

        };

        fileDataSource();

        ~fileDataSource() override;

        int Open(int flags) override;

        void Close() override;

        int64_t Seek(int64_t offset, int whence) override;

        size_t Read(void *buf, size_t nbyte) override;

    private:
        string mPath;
        int mFd;

    };
}


#endif //FRAMEWORK_FILEDATASOURCE_H
