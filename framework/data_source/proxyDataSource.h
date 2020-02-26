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

        void setImpl(readImpl read, seekImpl seek, openImpl open, interruptImpl interrupt, void *arg);

        void Interrupt(bool interrupt) override;
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

        void* mUserArg = nullptr;

    };
}


#endif //CICADA_PLAYER_PROXYDATASOURCE_H
