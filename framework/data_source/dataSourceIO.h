//
// Created by moqi on 2018/4/24.
//

#ifndef FRAMEWORK_DATA_SOURCE_UTILS_H
#define FRAMEWORK_DATA_SOURCE_UTILS_H
extern "C" {
#include <libavformat/avio.h>
};

#include "base/media/framework_type.h"
#include "IDataSource.h"

namespace Cicada{ ;

    class dataSourceIO {
    public:
        dataSourceIO(IDataSource *pDataSource);

        dataSourceIO(demuxer_callback_read read, demuxer_callback_seek seek, void *arg);

        ~dataSourceIO();

        int get_line(char *buf, int maxlen);

        int64_t seek(int64_t offset, int whence);

        bool isEOF();


    private:
        int init();

        static int read_callback(void *arg, uint8_t *buffer, int size);

        static int64_t seek_callback(void *arg, int64_t offset, int whence);

        IDataSource *mPDataSource{nullptr};
        AVIOContext *mPb = nullptr;
    };
}


#endif //FRAMEWORK_DATA_SOURCE_UTILS_H
