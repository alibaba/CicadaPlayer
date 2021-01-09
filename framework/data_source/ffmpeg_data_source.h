//
// Created by moqi on 2018/1/24.
//

#ifndef FRAMEWORK_FFMPEGDATASOURCE_H
#define FRAMEWORK_FFMPEGDATASOURCE_H
extern "C" {
#include <libavformat/url.h>
}

#include "IDataSource.h"
#include "dataSourcePrototype.h"
#if defined(WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

namespace Cicada {
    class ffmpegDataSource : public IDataSource, private dataSourcePrototype {
    public:
        static bool probe(const std::string &path)
        {
            return path.compare(0, 7, "rtmp://") == 0 || (access(path.c_str(), 0) == 0);
        };

        explicit ffmpegDataSource(const std::string &url);

        ~ffmpegDataSource() override;

        int Open(int flags) override;

        void Close() override;

        int64_t Seek(int64_t offset, int whence) override;

        int Read(void *buf, size_t nbyte) override;

        void Interrupt(bool interrupt) override;

        std::string Get_error_info(int error) override;

        std::string GetOption(const std::string &key) override;

    private:
        static int check_interrupt(void *pHandle);

    private:
        explicit ffmpegDataSource(int dummy) : IDataSource("")
        {
            addPrototype(this);
        }

        Cicada::IDataSource *clone(const std::string &uri) override
        {
            return new ffmpegDataSource(uri);
        };

        bool is_supported(const std::string &uri) override
        {
            return probe(uri);
        };

        static ffmpegDataSource se;

    private:
        AVIOContext *mPuc{};
        AVIOInterruptCB mInterruptCB{};
        int mInterrupted{};
        char mErrorMsg[AV_ERROR_MAX_STRING_SIZE]{};
    };
}


#endif //FRAMEWORK_FFMPEGDATASOURCE_H
