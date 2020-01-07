
#include <utils/frame_work_log.h>
#include "SourceReader.h"

namespace Cicada {

    SourceReader::SourceReader(shared_ptr<cachedSource> source)
        : mSource(source)
    {
    }

    SourceReader::~SourceReader()
    {
    }

    int64_t SourceReader::seek(int64_t offset, int whence)
    {
        switch (whence) {
            case SEEK_SIZE:
                return mSource->getFileSize();

            case SEEK_SET:
                if (offset >= 0) {
                    mPos = offset;
                    break;
                } else {
                    return -EINVAL;
                }

            case SEEK_END:
                if (mSource->getFileSize() < 0) {
                    return -(ENOSYS);
                }

                mPos = mSource->getFileSize() + offset;
                break;

            case SEEK_CUR:
                mPos += offset;
                break;

            default:
                return -EINVAL;
        }

        return mPos;
    }

    int SourceReader::read(void *buf, size_t nbyte)
    {
        int ret = mSource->readAt(buf, nbyte, mPos);

        if (ret > 0) {
            mPos += ret;
        }

        return ret;
    }
}
