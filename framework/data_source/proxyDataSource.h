//
// Created by moqi on 2019-04-18.
//

#ifndef CICADA_PLAYER_PROXYDATASOURCE_H
#define CICADA_PLAYER_PROXYDATASOURCE_H

#include "IDataSource.h"

typedef int (*readImpl)(void *arg, uint8_t *buffer, int size);

typedef int64_t (*seekImpl)(void *arg, int64_t offset, int whence);

typedef int (*openImpl)(void *arg, const char *url,int64_t start, int64_t end);

typedef void (*interruptImpl)(void *arg, int inter);

typedef void (*setSegmentListImpl)(void *arg, const std::vector<Cicada::mediaSegmentListEntry> &segments);

typedef int64_t (*getBufferDurationImpl)(void *arg, int index);

typedef void (*enableCacheImpl)(void *arg, const std::string &originUrl, bool enable);

namespace Cicada{
    class proxyDataSource : public IDataSource {
    public:
        proxyDataSource();

        ~proxyDataSource() override;

        int Open(int flags) override;

        int Open(const std::string &url) override;

        void Close() override;

        int64_t Seek(int64_t offset, int whence) override;

        int Read(void *buf, size_t nbyte) override;

        void setImpl(readImpl read, seekImpl seek, openImpl open, interruptImpl interrupt, setSegmentListImpl setSegmentList,
                     getBufferDurationImpl getBufferDuration, enableCacheImpl enableCache, void *arg);

        void Interrupt(bool interrupt) override;

        void setSegmentList(const std::vector<mediaSegmentListEntry> &segments) override;

        void enableCache(const std::string &originUrl, bool enable) override;

        int64_t getBufferDuration() override;

        //
        //        virtual string Get_error_info(int error);
        //
        //        virtual void Set_config(SourceConfig &config);
        //
        //        virtual void Get_config(SourceConfig &config);
        //
        //        virtual const string GetOption(const string &key);
        //
        //        virtual const string GetUri();

    private:
        readImpl mRead = nullptr;
        seekImpl mSeek = nullptr;
        openImpl mOpen = nullptr;
        interruptImpl mInter = nullptr;
        setSegmentListImpl mSetSegmentList = nullptr;
        getBufferDurationImpl mGetBufferDuration = nullptr;
        enableCacheImpl mEnableCache{nullptr};

        void* mUserArg = nullptr;

    };
}


#endif //CICADA_PLAYER_PROXYDATASOURCE_H
