
#ifndef SourceReader_H
#define SourceReader_H

#include "cachedSource.h"
#include <memory>

namespace Cicada{
    class SourceReader {
    public:
        explicit SourceReader(shared_ptr<cachedSource> source);

        ~SourceReader();

        int64_t seek(int64_t offset, int whence);

        int read(void *buf, size_t nbyte);

        IDataSource *getDataSource()
        {
            return mSource->getDataSource();
        }

    private:
        int64_t mPos = 0;

        shared_ptr<cachedSource> mSource;
    };
}

#endif //SourceReader_H
